#include "backward.hpp"
#include <csignal>

import std;
import congelado_heart;

/**
 * @brief Server process entry point — installs crash-signal handling, then hands off to
 * `ServerRunner` pointed at the sibling `plugins` directory (derived from the running binary's
 * own path) and a config path.
 * @warning The config path passed to `run()` is a hardcoded `"~/cc/congelado/config/
 * congelado.toml"` string, unlike the internal plugin directory which is genuinely derived from
 * `argv[0]`. There's no tilde-expansion happening anywhere in this call — the OS/filesystem
 * layer won't expand `~` for you — so this only ever finds the config on a checkout sitting at
 * exactly that path. Straight cooked for anyone running the binary from anywhere else or on
 * another machine.
 * @param argc argument count; only checked for `> 0` to decide whether `argv[0]` is safe to read.
 * @param argv argument vector; `argv[0]` is used to derive the internal plugins directory
 * relative to the running binary (should not normally be overridden). `argv[1]` (optional) is
 * the external (user-provided, custom) plugins directory, the intended user-facing knob.
 * @return whatever `ServerRunner::run` returns.
 */
int main(int argc, char *argv[]) {
    // Everything here can throw (SignalHandling's ctor, filesystem::path construction,
    // std::format, ServerRunner::run itself) and none of it was previously caught — an
    // uncaught exception escaping main() terminates the process the same way a noexcept
    // violation would, just with a worse diagnostic than a reported message.
    try {
        // Install crash-signal handling before anything else can go wrong.
        backward::SignalHandling sh;

        // A client (or the worker) closing its connection mid-write raises SIGPIPE on the next
        // write to that socket — default disposition kills the whole process, so every dropped
        // connection would otherwise take the server down. Ignore it; the write call itself
        // still reports the failure through errno/EPIPE.
        std::signal(SIGPIPE, SIG_IGN);

        // Derive the internal plugins directory relative to the running binary's own path —
        // falls back to an empty base if argv[0] somehow isn't there. Not a user-facing knob.
        auto base =
            argc > 0 ? std::filesystem::path(argv[0]).parent_path() : std::filesystem::path{};
        auto internal_plugin_dir =
            std::filesystem::path{std::format("{}/../../../plugins", base.string())};

        // argv[1] (optional) is the external, user-provided plugins directory.
        auto external_plugin_dir =
            argc > 1 ? std::optional<std::filesystem::path>{argv[1]} : std::nullopt;

        // Hand off to ServerRunner for the actual server main loop.
        return congelado::heart::ServerRunner{external_plugin_dir, internal_plugin_dir}
            .run("~/cc/congelado/config/congelado.toml");
    } catch (const std::exception &exception) {
        try {
            std::println(stderr, "fatal: {}", exception.what());
        } catch (...) { // NOLINT(bugprone-empty-catch) — best-effort diagnostic only
        }
        return 1;
    } catch (...) {
        try {
            std::println(stderr, "fatal: unknown exception");
        } catch (...) { // NOLINT(bugprone-empty-catch) — best-effort diagnostic only
        }
        return 1;
    }
}
