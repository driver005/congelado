module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module payload_local_plugin;

import congelado_plugin;
import interfaces;
import payload_local;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace {

/// @brief The local-disk payload-storage plugin — exports the PAYLOAD_STORAGE capability backed by
/// payload_local::LocalPayloadStorage. The storage member exists at plugin construction so the host
/// can resolve its pointer before build(); on_load re-roots it at the configured directory.
class LocalPayloadPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "payload_local"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override {
        return "payload_storage";
    }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_PAYLOAD_STORAGE;
    }

    /// @brief Re-roots the storage at the configured directory (default "payloads").
    /// @param host unused. @param cfg supplies the optional `directory` field.
    void on_load(CongeladoHostCallbacks const & /*host*/, CongeladoConfigView const &cfg) override {
        m_storage.set_directory(congelado::config_get(cfg, "directory").value_or("payloads"));
    }

    /// @brief Capability hook the host calls to get at this plugin's IExternalPayloadStorage surface.
    /// @return this plugin's LocalPayloadStorage, upcast to interfaces::IExternalPayloadStorage*.
    void *payload_storage_get() noexcept {
        return static_cast<interfaces::IExternalPayloadStorage *>(&m_storage);
    }

  private:
    payload_local::LocalPayloadStorage m_storage;
};

} // namespace

CONGELADO_PLUGIN(LocalPayloadPlugin);

#ifdef CONGELADO_TEST
namespace payload_local_plugin_tests {
using namespace boost::ut;

/// @brief A fresh, uniquely-named sandbox directory under the system temp dir, torn down when
/// the guard goes out of scope — same recipe as payload_local's own TempSandbox, duplicated
/// locally so this file's tests don't need to reach into that module's test-only namespace.
class TempSandbox {
  public:
    TempSandbox() {
        static std::atomic<std::uint64_t> counter{0};
        auto unique = std::format("congelado_payload_local_plugin_test_{:x}_{:x}",
                                  std::chrono::steady_clock::now().time_since_epoch().count(),
                                  counter.fetch_add(1, std::memory_order_relaxed));
        m_root = std::filesystem::temp_directory_path() / unique;
        std::filesystem::create_directories(m_root);
    }
    ~TempSandbox() {
        std::error_code error_code;
        std::filesystem::remove_all(m_root, error_code);
    }
    TempSandbox(const TempSandbox &) = delete;
    TempSandbox &operator=(const TempSandbox &) = delete;

    [[nodiscard]] std::filesystem::path get_root() const { return m_root; }

  private:
    std::filesystem::path m_root;
};

suite<"LocalPayloadPlugin"> local_payload_plugin_suite = [] {
    "identity/capabilities are the declared local payload-storage surface"_test = [] {
        LocalPayloadPlugin plugin;

        expect(plugin.get_name() == "payload_local");
        expect(plugin.get_version() == "1.0.0");
        expect(plugin.get_unique_type() == "payload_storage");
        expect(plugin.capabilities() == CONGELADO_CAP_PAYLOAD_STORAGE);
    };

    "payload_storage_get() exposes an IExternalPayloadStorage* that actually works after on_load re-roots the directory"_test =
        [] {
            TempSandbox sandbox;
            LocalPayloadPlugin plugin;

            const char *keys[] = {"directory"};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
            auto directory_value = sandbox.get_root().string();
            const char *values[] = {directory_value.c_str()};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
            CongeladoConfigView cfg{.keys = keys, .values = values, .count = 1};
            CongeladoHostCallbacks host{};

            plugin.on_load(host, cfg);

            auto *storage = static_cast<interfaces::IExternalPayloadStorage *>(plugin.payload_storage_get());
            expect(storage != nullptr) << fatal;

            std::string written_reference;
            storage->write(interfaces::PayloadType::TASK_OUTPUT, "plugin round-trip",
                          [&](std::string_view reference) { written_reference = std::string{reference}; });

            expect(!written_reference.empty()) << fatal;
            expect(written_reference.starts_with(sandbox.get_root().string()));

            std::string read_back;
            storage->read(written_reference, [&](std::string_view data) { read_back = std::string{data}; });
            expect(read_back == "plugin round-trip");
        };

    // SECURITY: reachable end-to-end through the exact capability surface the host actually
    // uses (payload_storage_get() -> IExternalPayloadStorage*), not just the internal class —
    // confirms the finding in local_storage.cppm's read() SECURITY comment is exploitable
    // through this plugin's real public interface, not merely a theoretical internal detail.
    "an absolute-path reference reaches straight through IExternalPayloadStorage::read() and past the configured storage directory"_test =
        [] {
            TempSandbox sandbox;
            auto storage_dir = sandbox.get_root() / "storage";
            auto outside_file = sandbox.get_root() / "outside" / "secret.txt";
            std::filesystem::create_directories(outside_file.parent_path());
            {
                std::ofstream out{outside_file, std::ios::binary};
                out << "TOP-SECRET-VIA-PLUGIN-SURFACE";
            }

            LocalPayloadPlugin plugin;
            const char *keys[] = {"directory"};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
            auto directory_value = storage_dir.string();
            const char *values[] = {directory_value.c_str()};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
            CongeladoConfigView cfg{.keys = keys, .values = values, .count = 1};
            CongeladoHostCallbacks host{};
            plugin.on_load(host, cfg);

            auto *storage = static_cast<interfaces::IExternalPayloadStorage *>(plugin.payload_storage_get());
            expect(storage != nullptr) << fatal;

            std::string read_back;
            storage->read(outside_file.string(), [&](std::string_view data) { read_back = std::string{data}; });

            expect(read_back == "TOP-SECRET-VIA-PLUGIN-SURFACE");
        };

    "on_load without a configured 'directory' key falls back to the default without crashing"_test =
        [] {
            LocalPayloadPlugin plugin;
            CongeladoConfigView cfg{};
            CongeladoHostCallbacks host{};

            plugin.on_load(host, cfg);

            auto *storage = static_cast<interfaces::IExternalPayloadStorage *>(plugin.payload_storage_get());
            expect(storage != nullptr) << fatal;
        };
};

} // namespace payload_local_plugin_tests
#endif
