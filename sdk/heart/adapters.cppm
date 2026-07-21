module;

#include <congelado/abi.h>

export module congelado_heart:adapters;

import std;
import interfaces;
import core_logger;
import core_plugin;

namespace congelado::heart {

using core::plugin::types::PluginRef;

class LoggerAdapter final : public interfaces::ILogger,
                            public std::enable_shared_from_this<LoggerAdapter> {
  public:
    using LoggerWriteFn = void (*)(int, const char *, size_t) noexcept;

    /**
     * @brief Wraps a plugin's `congelado_logger_write` C symbol in an ILogger so the host-side
     * registry can log through it like any other sink — bridges the ABI gap, no cap.
     * @param name the logger's display name, usually the owning plugin's name.
     * @param write_fn the plugin's exported C write function this adapter forwards every call to.
     */
    explicit LoggerAdapter(std::string name, LoggerWriteFn write_fn)
        : m_name{std::move(name)}, m_write{write_fn} {}

    /// @brief Gets this adapter's display name. @return the logger's name, motion.
    [[nodiscard]] std::string_view get_name() const noexcept override { return m_name; }

    /**
     * @brief Forwards a log line straight to the plugin's C write symbol.
     * @param level the severity of the line being logged.
     * @param msg the text getting logged.
     */
    void write(interfaces::LogLevel level, std::string_view msg) noexcept override {
        m_write(static_cast<int>(level), msg.data(), msg.size());
    }

    /**
     * @brief Shortcut for logging at Error severity (hardcoded level `4`) without spelling it
     * out — that's an L and you already know it.
     * @param msg the error text getting logged.
     */
    void error(std::string_view msg) noexcept override { m_write(4, msg.data(), msg.size()); }

    /**
     * @brief Hands this adapter off to `LoggerRegistry` so the rest of the host can log through
     * it — grabs a `shared_from_this()`, so the adapter must already be owned by a `shared_ptr`
     * before this gets called, otherwise it's straight UB.
     */
    void register_logger() {
        auto self = shared_from_this();
        core::logger::LoggerRegistry::register_logger(self);
    }

    /**
     * @brief Builds a LoggerAdapter from a loaded plugin's exported logger capability, if it
     * actually has one — checks the capability bitmask before touching anything else.
     * @warning Bails to `nullptr` on any missing piece (no `congelado_capabilities` symbol, the
     * `CONGELADO_CAP_LOGGER` bit unset, or no `congelado_logger_write` symbol) — callers must
     * null-check before dereferencing, skipping that check is a straight L waiting to happen.
     * @param ref the loaded plugin's symbol table to pull the logger capability out of.
     * @return a live LoggerAdapter if the plugin exports the logger capability, `nullptr` otherwise.
     */
    static std::shared_ptr<LoggerAdapter> register_from(PluginRef &ref) {
        // No capabilities symbol at all means this plugin never opted into anything — bail.
        auto cap_it = ref.m_data.find("congelado_capabilities");
        if (cap_it == ref.m_data.end()) {
            return nullptr;
        }

        // Capabilities symbol exists but the logger bit isn't set — not our concern here.
        auto caps = std::any_cast<uint32_t>(cap_it->second);
        if ((caps & 1U) == 0) { // CONGELADO_CAP_LOGGER
            return nullptr;
        }

        // Bit's set but no write symbol resolved — can't build a working adapter without it.
        auto write_it = ref.m_data.find("congelado_logger_write");
        if (write_it == ref.m_data.end()) {
            return nullptr;
        }

        auto *raw = std::any_cast<void *>(write_it->second);
        auto write_fn = reinterpret_cast<LoggerWriteFn>(raw);  // FIXME(clang-tidy): reinterpret_cast usage — cross-ABI cast of a dlsym'd void* back to its known function pointer type

        // Plugin name is nice-to-have, not required — fall back to a placeholder if missing.
        std::string pname;
        if (auto name_it = ref.m_data.find("congelado_plugin_name"); name_it != ref.m_data.end()) {
            pname = std::any_cast<const std::string &>(name_it->second);
        } else {
            pname = "unnamed";
        }

        return std::make_shared<LoggerAdapter>(std::move(pname), write_fn);
    }

  private:
    std::string m_name;
    LoggerWriteFn m_write;
};

} // namespace congelado::heart
