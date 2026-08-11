module;

#include "congelado/abi.h"

export module congelado_plugin;

import std;
import interfaces;
export import core_plugin;
export namespace congelado {

class Plugin : public interfaces::ILogger {
  public:
    /// @brief Default-constructs a Plugin — subclasses set up real state in their own ctors.
    Plugin() noexcept = default;
    /// @brief Virtual dtor, default's fine — no owned resources at this base-class level.
    ~Plugin() override = default;
    /// @brief Deleted — a live Plugin instance is owned singleton-style by the CONGELADO_PLUGIN
    /// macro's static pointer, no copying that motion.
    Plugin(const Plugin &) = delete;
    /// @brief Deleted — same reason as the copy ctor.
    Plugin &operator=(const Plugin &) = delete;
    /// @brief Deleted — same reason as the copy ctor, moving a singleton-owned instance makes
    /// no sense either.
    Plugin(Plugin &&) = delete;
    /// @brief Deleted — same reason as the move ctor.
    Plugin &operator=(Plugin &&) = delete;

    /**
     * @brief Tells the host which plugin this is — every subclass must override this, it's the
     * name that shows up wherever plugins get listed or logged.
     * @return the plugin's name.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override = 0;
    /**
     * @brief Tells the host which version this plugin build is — pure virtual, every subclass
     * has to report something real here.
     * @return the plugin's version string.
     */
    [[nodiscard]] virtual std::string_view get_version() const noexcept = 0;
    /**
     * @brief Tells the host which optional capabilities (logger, protocol, storage, custom)
     * this plugin exports, as an OR'd bitmask of the `CONGELADO_CAP_*` macros.
     * @note Defaults to `0` (no capabilities) — override this to opt into `_cap_dispatch`
     * routing for `logger_write`/`protocol_get`/`storage_get`, otherwise the host never bothers
     * calling those hooks at all.
     * @return the capability bitmask, `0` by default.
     */
    [[nodiscard]] virtual std::uint32_t capabilities() const noexcept { return 0; }
    /**
     * @brief Optional extra type tag beyond `get_type()`, for plugins that need finer-grained
     * identity than "plugin" vs "worker" (e.g. disambiguating multiple protocol plugins).
     * @return the unique type string, empty by default.
     */
    [[nodiscard]] virtual std::string_view get_unique_type() const noexcept { return {}; }
    /**
     * @brief Tells the host the broad plugin category — mirrors the `congelado_type()` C ABI
     * symbol.
     * @return `"plugin"` by default; workers override this via a different code path entirely.
     */
    [[nodiscard]] virtual std::string_view get_type() const noexcept { return "plugin"; }
    /**
     * @brief Lists the other plugin types this one depends on being already loaded — no motion
     * for this plugin until its requirements are already up.
     * @return a span of required type names, empty by default (no dependencies).
     */
    [[nodiscard]] virtual std::span<const std::string_view> get_requires() const noexcept { return {}; }
    /**
     * @brief Lists plugin types that must load *after* this one — the inverse ordering
     * constraint from `get_requires()`.
     * @return a span of type names this plugin must precede, empty by default.
     */
    [[nodiscard]] virtual std::span<const std::string_view> get_load_before_types() const noexcept { return {}; }

    /**
     * @brief Tells the host which worker task type this plugin executes, if it's acting as a
     * worker.
     * @return the worker type string, empty by default (not a worker).
     */
    [[nodiscard]] virtual std::string_view get_worker_type() const noexcept { return {}; }
    /**
     * @brief Runs this plugin's worker task against the given input, if it has one.
     * @note Default no-op returns an empty view — only plugins that override `get_worker_type()`
     * with a non-empty value are expected to actually do something here.
     * @param input the task's config key/value input view; unnamed param, unused in the default.
     * @return the task's config key/value output view, empty by default.
     */
    [[nodiscard]] virtual CongeladoConfigView execute_worker(
        const CongeladoConfigView * /*input*/) {
        return {};
    }

    /**
     * @brief Fires once, right after the host resolves the plugin's C ABI symbols — the spot to
     * stash the host callback table and read initial config.
     * @throws Whatever the override throws; `CONGELADO_PLUGIN`'s generated `congelado_init`
     * catches everything and turns it into a `-1` return, so throwing here is safe but the
     * exception itself never reaches the host.
     * @param host unnamed param, unused in the default — the host callback table (router,
     * controller, leverager context pointers, log/schedule fns).
     * @param cfg unnamed param, unused in the default — this plugin's parsed config view.
     */
    virtual void on_load(CongeladoHostCallbacks const & /*host*/,
                         CongeladoConfigView const & /*cfg*/) {}
    /// @brief Fires right before the plugin instance gets destroyed — default no-op, override
    /// to release resources ahead of teardown, otherwise whatever you were holding just leaks.
    virtual void on_unload() noexcept {}
    /// @brief Fires when the host catches SIGINT/SIGTERM, before full plugin teardown starts —
    /// default no-op. Protocol plugins override this to stop accepting new connections and close
    /// every live one immediately, instead of waiting for `on_unload()` at full process teardown.
    virtual void on_shutdown_requested() noexcept {}
    /// @brief Fires once every plugin has finished loading — default no-op, override for
    /// cross-plugin setup that needs everything already wired up, lowkey a "we're all here now"
    /// signal.
    virtual void on_ready() noexcept {}
    /**
     * @brief Asked whenever the host wants to hot-reload this plugin — gives it veto power, so
     * a plugin mid-critical-section can say "not yet, bet" instead of getting yanked.
     * @return `true` to allow the reload (the default), `false` to block it.
     */
    [[nodiscard]] virtual bool on_reload_requested() noexcept { return true; }

    /**
     * @brief The actual log sink a subclass overrides to receive lines routed through
     * `write()`/`error()` — this is what `_cap_dispatch::logger_write` calls when
     * `CONGELADO_CAP_LOGGER` is set.
     * @note Default no-op — a plugin that never sets the logger capability bit can leave this
     * alone entirely, nothing ever calls it.
     * @param level unnamed param, unused in the default — the log level as a raw int (matches
     * `interfaces::LogLevel`'s underlying value).
     * @param msg unnamed param, unused in the default — the text being logged.
     */
    virtual void logger_write(int /*level*/, std::string_view /*msg*/) noexcept {}
    /**
     * @brief Implements `ILogger::write` by forwarding straight into `logger_write` — `final`,
     * so subclasses hook logging by overriding `logger_write` instead, not this.
     * @param level the log level being forwarded.
     * @param msg the text being logged.
     */
    void write(interfaces::LogLevel level, std::string_view msg) noexcept final {
        logger_write(static_cast<int>(level), msg);
    }
    /**
     * @brief Implements `ILogger::error` by forwarding to `logger_write` at hardcoded level `4`
     * — `final`, same deal as `write`.
     * @param msg the error text being logged.
     */
    void error(std::string_view msg) noexcept final { logger_write(4, msg); }
};

namespace _cap_dispatch {

template <typename T>
concept has_logger_write = requires(T *plugin) { plugin->logger_write(int{}, std::string_view{}); };

template <typename T>
void logger_write(T *plugin, int level, std::string_view msg) noexcept {
    if constexpr (has_logger_write<T>) {
        plugin->logger_write(level, msg);
    }
}

template <typename T>
concept has_protocol_get = requires(T *plugin) { plugin->protocol_get(); };

template <typename T>
void *protocol_get(T *plugin) noexcept {
    if constexpr (has_protocol_get<T>) {
        return plugin->protocol_get();
    }
    return nullptr;
}

template <typename T>
concept has_storage_get = requires(T *plugin) { plugin->storage_get(); };

template <typename T>
void *storage_get(T *plugin) noexcept {
    if constexpr (has_storage_get<T>) {
        return plugin->storage_get();
    }
    return nullptr;
}

template <typename T>
concept has_serde_get = requires(T *plugin) { plugin->serde_get(); };

template <typename T>
void *serde_get(T *plugin) noexcept {
    if constexpr (has_serde_get<T>) {
        return plugin->serde_get();
    }
    return nullptr;
}

template <typename T>
concept has_otel_get = requires(T *plugin) { plugin->otel_get(); };

template <typename T>
void *otel_get(T *plugin) noexcept {
    if constexpr (has_otel_get<T>) {
        return plugin->otel_get();
    }
    return nullptr;
}

template <typename T>
concept has_bridge_get = requires(T *plugin) { plugin->bridge_get(); };

template <typename T>
void *bridge_get(T *plugin) noexcept {
    if constexpr (has_bridge_get<T>) {
        return plugin->bridge_get();
    }
    return nullptr;
}

template <typename T>
concept has_openapi_get = requires(T *plugin) { plugin->openapi_get(); };

template <typename T>
void *openapi_get(T *plugin) noexcept {
    if constexpr (has_openapi_get<T>) {
        return plugin->openapi_get();
    }
    return nullptr;
}

template <typename T>
concept has_search_get = requires(T *plugin) { plugin->search_get(); };

template <typename T>
void *search_get(T *plugin) noexcept {
    if constexpr (has_search_get<T>) {
        return plugin->search_get();
    }
    return nullptr;
}

template <typename T>
concept has_event_get = requires(T *plugin) { plugin->event_get(); };

template <typename T>
void *event_get(T *plugin) noexcept {
    if constexpr (has_event_get<T>) {
        return plugin->event_get();
    }
    return nullptr;
}

template <typename T>
concept has_cache_get = requires(T *plugin) { plugin->cache_get(); };

template <typename T>
void *cache_get(T *plugin) noexcept {
    if constexpr (has_cache_get<T>) {
        return plugin->cache_get();
    }
    return nullptr;
}

template <typename T>
concept has_cron_get = requires(T *plugin) { plugin->cron_get(); };

template <typename T>
void *cron_get(T *plugin) noexcept {
    if constexpr (has_cron_get<T>) {
        return plugin->cron_get();
    }
    return nullptr;
}

template <typename T>
concept has_bridge_native_handle = requires(T *plugin) { plugin->bridge_native_handle(); };

template <typename T>
void *bridge_native_handle(T *plugin) noexcept {
    if constexpr (has_bridge_native_handle<T>) {
        return plugin->bridge_native_handle();
    }
    return nullptr;
}

/**
 * @brief The universal plugin-call dispatcher — the macro-generated `congelado_call` symbol
 * forwards every capability call here instead of exposing one C symbol per capability. GET
 * actions (SERDE/STORAGE/PROTOCOL) hand back a raw interface pointer the host casts and calls
 * virtuals on directly, in-process — same as the old storage_get/protocol_get. WRITE/ERROR
 * (LOGGER) carry real marshaled data since ILogger is deliberately kept non-templated/ABI-stable.
 */
template <typename T>
CongeladoAny call(T *plugin, CongeladoRunType type, CongeladoRunAction action,
                  const CongeladoAny *args, std::size_t args_count) noexcept {
    switch (type) {
    case CONGELADO_RUN_LOGGER:
        if (action == CONGELADO_ACTION_WRITE && args_count >= 2) {
            logger_write(plugin, static_cast<int>(args[0].v_int64), std::string_view{args[1].v_cstr});
        } else if (action == CONGELADO_ACTION_ERROR && args_count >= 1) {
            logger_write(plugin, 4, std::string_view{args[0].v_cstr});
        }
        return CongeladoAny{};
    case CONGELADO_RUN_STORAGE:
        return CongeladoAny{.type_index = CG_PTR, .zero_padding = 0, .v_ptr = storage_get(plugin)};
    case CONGELADO_RUN_PROTOCOL:
        return CongeladoAny{.type_index = CG_PTR, .zero_padding = 0, .v_ptr = protocol_get(plugin)};
    case CONGELADO_RUN_SERDE:
        return CongeladoAny{.type_index = CG_PTR, .zero_padding = 0, .v_ptr = serde_get(plugin)};
    case CONGELADO_RUN_OTEL:
        return CongeladoAny{.type_index = CG_PTR, .zero_padding = 0, .v_ptr = otel_get(plugin)};
    case CONGELADO_RUN_OPENAPI:
        return CongeladoAny{.type_index = CG_PTR, .zero_padding = 0, .v_ptr = openapi_get(plugin)};
    case CONGELADO_RUN_SEARCH:
        return CongeladoAny{.type_index = CG_PTR, .zero_padding = 0, .v_ptr = search_get(plugin)};
    case CONGELADO_RUN_EVENTS:
        return CongeladoAny{.type_index = CG_PTR, .zero_padding = 0, .v_ptr = event_get(plugin)};
    case CONGELADO_RUN_CACHE:
        return CongeladoAny{.type_index = CG_PTR, .zero_padding = 0, .v_ptr = cache_get(plugin)};
    case CONGELADO_RUN_CRON:
        return CongeladoAny{.type_index = CG_PTR, .zero_padding = 0, .v_ptr = cron_get(plugin)};
    case CONGELADO_RUN_BRIDGE:
        if (action == CONGELADO_ACTION_GET_NATIVE_HANDLE) {
            return CongeladoAny{.type_index = CG_PTR, .zero_padding = 0,
                                .v_ptr = bridge_native_handle(plugin)};
        }
        return CongeladoAny{.type_index = CG_PTR, .zero_padding = 0, .v_ptr = bridge_get(plugin)};
    }
    return CongeladoAny{};
}

} // namespace _cap_dispatch

// ── Re-exports ───────────────────────────────────────────────────────────────

using core::plugin::types::router_ctx;
using core::plugin::types::controller_ctx;
using core::plugin::types::registry_ctx;
using core::plugin::types::leverager_ctx;
using core::plugin::types::database_ctx;
using core::plugin::types::lua_bridge_ctx;
using core::plugin::types::search_ctx;
using core::plugin::types::cache_ctx;
using core::plugin::types::cron_ctx;
using core::plugin::types::config_get;
using core::plugin::types::config_for_each;
using core::plugin::types::ConfigViewBuilder;
using FfiRuntime = core::plugin::FfiRuntime;

using GenerationConfig = core::plugin::types::GenerationConfig;
using PythonConfig = core::plugin::types::PythonConfig;
using LuaConfig = core::plugin::types::LuaConfig;

// Value types
using Value = core::plugin::Value;
using None = core::plugin::None;
using Int = core::plugin::Int;
using Float = core::plugin::Float;
using Bool = core::plugin::Bool;
using Str = core::plugin::Str;
using Map = core::plugin::Map;
using Array = core::plugin::Array;
template <typename T>
using ValueTraits = core::plugin::ValueTraits<T>;

} // namespace congelado
