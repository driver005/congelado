module;

#include <congelado/abi.h>

export module congelado_heart:adapters;

import std;
import interfaces;
import core_logger;
import core_otel;
import core_plugin;
import utils_openapi; // for interfaces::IOpenApiGenerator (see utils_openapi:generator_interface's
                       // own doc comment for why it lives there instead of in `interfaces` itself)

export namespace congelado::heart {

using core::plugin::types::PluginRef;

// Every host-side adapter below resolves the plugin's single `congelado_call` symbol and
// checks a capability bit before touching anything else — this shared helper does both,
// bailing to nullptr on any missing piece (no `congelado_capabilities` symbol, the bit unset,
// or no `congelado_call` symbol resolved). Callers must null-check the result.
[[nodiscard]] inline congelado_call_fn resolve_call_fn(PluginRef &ref, std::uint32_t cap_bit) {
    auto cap_it = ref.m_data.find("congelado_capabilities");
    if (cap_it == ref.m_data.end()) {
        return nullptr;
    }
    auto caps = std::any_cast<std::uint32_t>(cap_it->second);
    if ((caps & cap_bit) == 0) {
        return nullptr;
    }
    auto call_it = ref.m_data.find("congelado_call");
    if (call_it == ref.m_data.end()) {
        return nullptr;
    }
    auto *raw = std::any_cast<void *>(call_it->second);
    return reinterpret_cast<congelado_call_fn>(raw);  // FIXME(clang-tidy): reinterpret_cast usage — cross-ABI cast of a dlsym'd void* back to its known function pointer type
}

class LoggerAdapter final : public interfaces::ILogger,
                            public std::enable_shared_from_this<LoggerAdapter> {
  public:
    /**
     * @brief Wraps a plugin's universal `congelado_call` symbol in an ILogger so the host-side
     * registry can log through it like any other sink — bridges the ABI gap, no cap.
     * @param name the logger's display name, usually the owning plugin's name.
     * @param call_fn the plugin's exported universal call symbol this adapter routes
     * LOGGER/WRITE and LOGGER/ERROR calls through.
     */
    explicit LoggerAdapter(std::string name, congelado_call_fn call_fn)
        : m_name{std::move(name)}, m_call{call_fn} {}

    /// @brief Gets this adapter's display name. @return the logger's name, motion.
    [[nodiscard]] std::string_view get_name() const noexcept override { return m_name; }

    /**
     * @brief Forwards a log line to the plugin via `congelado_call(LOGGER, WRITE, ...)`.
     * @note `msg` gets copied into an owned, null-terminated string first — any `CG_STR`
     * crossing this ABI is expected to be null-terminated (matches how `STRING_FN` symbols
     * already behave), and an arbitrary `string_view` isn't guaranteed to be.
     * @param level the severity of the line being logged.
     * @param msg the text getting logged.
     */
    void write(interfaces::LogLevel level, std::string_view msg) noexcept override {
        std::string owned{msg};
        CongeladoAny args[2]{};
        args[0].type_index = CG_INT;
        args[0].v_int64 = static_cast<std::int64_t>(level);
        args[1].type_index = CG_STR;
        args[1].v_cstr = owned.c_str();
        m_call(CONGELADO_RUN_LOGGER, CONGELADO_ACTION_WRITE, args, 2);
    }

    /**
     * @brief Shortcut for logging at Error severity via `congelado_call(LOGGER, ERROR, ...)`.
     * @param msg the error text getting logged.
     */
    void error(std::string_view msg) noexcept override {
        std::string owned{msg};
        CongeladoAny args[1]{};
        args[0].type_index = CG_STR;
        args[0].v_cstr = owned.c_str();
        m_call(CONGELADO_RUN_LOGGER, CONGELADO_ACTION_ERROR, args, 1);
    }

    /**
     * @brief Hands this adapter off to `registry` so the rest of the host can log through it —
     * grabs a `shared_from_this()`, so the adapter must already be owned by a `shared_ptr`
     * before this gets called, otherwise it's straight UB.
     * @param registry the (already-active) registry to add this adapter to.
     */
    void register_logger(core::logger::LoggerRegistry &registry) {
        auto self = shared_from_this();
        registry.add_logger(self);
    }

    /**
     * @brief Builds a LoggerAdapter from a loaded plugin's exported logger capability, if it
     * actually has one — checks the capability bitmask before touching anything else.
     * @param ref the loaded plugin's symbol table to pull the logger capability out of.
     * @return a live LoggerAdapter if the plugin exports the logger capability, `nullptr`
     * otherwise.
     */
    static std::shared_ptr<LoggerAdapter> register_from(PluginRef &ref) {
        auto call_fn = resolve_call_fn(ref, 1U); // CONGELADO_CAP_LOGGER
        if (call_fn == nullptr) {
            return nullptr;
        }

        // Plugin name is nice-to-have, not required — fall back to a placeholder if missing.
        std::string pname;
        if (auto name_it = ref.m_data.find("congelado_plugin_name"); name_it != ref.m_data.end()) {
            pname = std::any_cast<const std::string &>(name_it->second);
        } else {
            pname = "unnamed";
        }

        return std::make_shared<LoggerAdapter>(std::move(pname), call_fn);
    }

  private:
    std::string m_name;
    congelado_call_fn m_call;
};

/**
 * @brief Resolves a loaded plugin's SERDE-capability interface pointer via
 * `congelado_call(SERDE, GET, ...)`, if it has one.
 * @param ref the loaded plugin's symbol table.
 * @return a non-owning `shared_ptr<interfaces::ISerdeFormat>` (the plugin singleton owns the
 * real instance — this wraps it with a no-op deleter, same lifetime idiom `storage_get`'s
 * caller would need), or `nullptr` if the plugin doesn't export the SERDE capability.
 */
[[nodiscard]] inline std::shared_ptr<interfaces::ISerdeFormat> resolve_serde_format(PluginRef &ref) {
    auto call_fn = resolve_call_fn(ref, 16U); // CONGELADO_CAP_SERDE
    if (call_fn == nullptr) {
        return nullptr;
    }
    auto result = call_fn(CONGELADO_RUN_SERDE, CONGELADO_ACTION_GET, nullptr, 0);
    if (result.type_index != CG_PTR || result.v_ptr == nullptr) {
        return nullptr;
    }
    auto *format = static_cast<interfaces::ISerdeFormat *>(result.v_ptr);
    return std::shared_ptr<interfaces::ISerdeFormat>(format, [](interfaces::ISerdeFormat *) {});
}

/**
 * @brief Resolves a loaded plugin's STORAGE-capability interface pointer via
 * `congelado_call(STORAGE, GET, ...)`, if it has one.
 * @param ref the loaded plugin's symbol table.
 * @return a non-owning `shared_ptr<interfaces::IDatabase>` (same lifetime idiom as
 * `resolve_serde_format`), or `nullptr` if the plugin doesn't export the STORAGE capability.
 */
[[nodiscard]] inline std::shared_ptr<interfaces::IDatabase> resolve_storage(PluginRef &ref) {
    auto call_fn = resolve_call_fn(ref, 4U); // CONGELADO_CAP_STORAGE
    if (call_fn == nullptr) {
        return nullptr;
    }
    auto result = call_fn(CONGELADO_RUN_STORAGE, CONGELADO_ACTION_GET, nullptr, 0);
    if (result.type_index != CG_PTR || result.v_ptr == nullptr) {
        return nullptr;
    }
    auto *database = static_cast<interfaces::IDatabase *>(result.v_ptr);
    return std::shared_ptr<interfaces::IDatabase>(database, [](interfaces::IDatabase *) {});
}

/**
 * @brief Resolves a loaded plugin's SEARCH-capability interface pointer via
 * `congelado_call(SEARCH, GET, ...)`, if it has one.
 * @param ref the loaded plugin's symbol table.
 * @return a non-owning `shared_ptr<interfaces::ISearchProvider>` (same lifetime idiom as
 * `resolve_serde_format`), or `nullptr` if the plugin doesn't export the SEARCH capability.
 */
[[nodiscard]] inline std::shared_ptr<interfaces::ISearchProvider> resolve_search_provider(PluginRef &ref) {
    auto call_fn = resolve_call_fn(ref, 256U); // CONGELADO_CAP_SEARCH
    if (call_fn == nullptr) {
        return nullptr;
    }
    auto result = call_fn(CONGELADO_RUN_SEARCH, CONGELADO_ACTION_GET, nullptr, 0);
    if (result.type_index != CG_PTR || result.v_ptr == nullptr) {
        return nullptr;
    }
    auto *provider = static_cast<interfaces::ISearchProvider *>(result.v_ptr);
    return std::shared_ptr<interfaces::ISearchProvider>(provider, [](interfaces::ISearchProvider *) {});
}

/**
 * @brief Resolves a loaded plugin's CACHE-capability interface pointer via
 * `congelado_call(CACHE, GET, ...)`, if it has one.
 * @param ref the loaded plugin's symbol table.
 * @return a non-owning `shared_ptr<interfaces::ICache>` (same lifetime idiom as
 * `resolve_serde_format`), or `nullptr` if the plugin doesn't export the CACHE capability.
 */
[[nodiscard]] inline std::shared_ptr<interfaces::ICache> resolve_cache(PluginRef &ref) {
    auto call_fn = resolve_call_fn(ref, 1024U); // CONGELADO_CAP_CACHE
    if (call_fn == nullptr) {
        return nullptr;
    }
    auto result = call_fn(CONGELADO_RUN_CACHE, CONGELADO_ACTION_GET, nullptr, 0);
    if (result.type_index != CG_PTR || result.v_ptr == nullptr) {
        return nullptr;
    }
    auto *cache = static_cast<interfaces::ICache *>(result.v_ptr);
    return std::shared_ptr<interfaces::ICache>(cache, [](interfaces::ICache *) {});
}

/**
 * @brief Resolves a loaded plugin's CRON-capability interface pointer via
 * `congelado_call(CRON, GET, ...)`, if it has one.
 * @param ref the loaded plugin's symbol table.
 * @return a non-owning `shared_ptr<interfaces::ICron>` (same lifetime idiom as
 * `resolve_serde_format`), or `nullptr` if the plugin doesn't export the CRON capability.
 */
[[nodiscard]] inline std::shared_ptr<interfaces::ICron> resolve_cron_provider(PluginRef &ref) {
    auto call_fn = resolve_call_fn(ref, 2048U); // CONGELADO_CAP_CRON
    if (call_fn == nullptr) {
        return nullptr;
    }
    auto result = call_fn(CONGELADO_RUN_CRON, CONGELADO_ACTION_GET, nullptr, 0);
    if (result.type_index != CG_PTR || result.v_ptr == nullptr) {
        return nullptr;
    }
    auto *cron = static_cast<interfaces::ICron *>(result.v_ptr);
    return std::shared_ptr<interfaces::ICron>(cron, [](interfaces::ICron *) {});
}

/**
 * @brief Resolves a loaded plugin's EVENTS-capability interface pointer via
 * `congelado_call(EVENTS, GET, ...)`, if it has one.
 * @param ref the loaded plugin's symbol table.
 * @return a non-owning `shared_ptr<interfaces::IEventSink>` (same lifetime idiom as
 * `resolve_serde_format`), or `nullptr` if the plugin doesn't export the EVENTS capability.
 */
[[nodiscard]] inline std::shared_ptr<interfaces::IEventSink> resolve_event_sink(PluginRef &ref) {
    auto call_fn = resolve_call_fn(ref, 512U); // CONGELADO_CAP_EVENTS
    if (call_fn == nullptr) {
        return nullptr;
    }
    auto result = call_fn(CONGELADO_RUN_EVENTS, CONGELADO_ACTION_GET, nullptr, 0);
    if (result.type_index != CG_PTR || result.v_ptr == nullptr) {
        return nullptr;
    }
    auto *sink = static_cast<interfaces::IEventSink *>(result.v_ptr);
    return std::shared_ptr<interfaces::IEventSink>(sink, [](interfaces::IEventSink *) {});
}

/**
 * @brief Resolves a loaded plugin's OTEL-capability interface pointer via
 * `congelado_call(OTEL, GET, ...)`, if it has one.
 * @param ref the loaded plugin's symbol table.
 * @return a non-owning `shared_ptr<interfaces::IOtelProvider>` (same lifetime idiom as
 * `resolve_serde_format`), or `nullptr` if the plugin doesn't export the OTEL capability. A
 * returned provider may still answer `nullptr` from any of its own `get_tracer_provider()`/
 * `get_meter_provider()`/`get_log_provider()` accessors — a plugin can support any subset of the
 * three OTel signals.
 */
[[nodiscard]] inline std::shared_ptr<interfaces::IOtelProvider> resolve_otel_provider(PluginRef &ref) {
    auto call_fn = resolve_call_fn(ref, 64U); // CONGELADO_CAP_OTEL
    if (call_fn == nullptr) {
        return nullptr;
    }
    auto result = call_fn(CONGELADO_RUN_OTEL, CONGELADO_ACTION_GET, nullptr, 0);
    if (result.type_index != CG_PTR || result.v_ptr == nullptr) {
        return nullptr;
    }
    auto *provider = static_cast<interfaces::IOtelProvider *>(result.v_ptr);
    return std::shared_ptr<interfaces::IOtelProvider>(provider, [](interfaces::IOtelProvider *) {});
}

/**
 * @brief Resolves a loaded plugin's OPENAPI-capability interface pointer via
 * `congelado_call(OPENAPI, GET, ...)`, if it has one.
 * @param ref the loaded plugin's symbol table.
 * @return a non-owning `shared_ptr<interfaces::IOpenApiGenerator>` (same lifetime idiom as
 * `resolve_serde_format`), or `nullptr` if the plugin doesn't export the OPENAPI capability.
 */
[[nodiscard]] inline std::shared_ptr<interfaces::IOpenApiGenerator>
resolve_openapi_generator(PluginRef &ref) {
    auto call_fn = resolve_call_fn(ref, 128U); // CONGELADO_CAP_OPENAPI
    if (call_fn == nullptr) {
        return nullptr;
    }
    auto result = call_fn(CONGELADO_RUN_OPENAPI, CONGELADO_ACTION_GET, nullptr, 0);
    if (result.type_index != CG_PTR || result.v_ptr == nullptr) {
        return nullptr;
    }
    auto *generator = static_cast<interfaces::IOpenApiGenerator *>(result.v_ptr);
    return std::shared_ptr<interfaces::IOpenApiGenerator>(generator,
                                                          [](interfaces::IOpenApiGenerator *) {});
}

/**
 * @brief Bridges the *existing* `core::logger` fan-out into OTel logs — registers into
 * `core::logger::LoggerRegistry` exactly like any other logger backend (e.g. `LoggerAdapter`),
 * so every one of the ~448 existing `core::logger::*` call sites gets an OTel log record for
 * free, with zero changes to those call sites. Reads `core::otel::current_context()` for
 * trace/span correlation at the moment each line is logged, then fans the resulting
 * `interfaces::LogRecord` out to every provider in `core::otel::LogRecordRegistry`.
 */
class OtelLogBridge final : public interfaces::ILogger,
                            public std::enable_shared_from_this<OtelLogBridge> {
  public:
    /// @brief Gets this bridge's display name. @return a fixed identifying name.
    [[nodiscard]] std::string_view get_name() const noexcept override { return "otel-log-bridge"; }

    /**
     * @brief Forwards a log line as an OTel log record.
     * @param level the severity of the line being logged.
     * @param msg the text getting logged.
     */
    void write(interfaces::LogLevel level, std::string_view msg) noexcept override {
        emit(to_otel_severity(level), msg);
    }

    /**
     * @brief Shortcut for logging at Error severity, matching `ILogger::error`'s contract.
     * @param msg the error text getting logged.
     */
    void error(std::string_view msg) noexcept override {
        emit(interfaces::LogSeverity::ERROR, msg);
    }

    /**
     * @brief Hands this bridge off to `registry` so every fanned-out log call also reaches OTel
     * — grabs a `shared_from_this()`, so the bridge must already be owned by a `shared_ptr`
     * before this gets called, otherwise it's straight UB.
     * @param registry the (already-active) logger registry to add this bridge to.
     */
    void register_bridge(core::logger::LoggerRegistry &registry) {
        registry.add_logger(shared_from_this());
    }

    /**
     * @brief Constructs a fresh `OtelLogBridge` and registers it into `registry` in one call.
     * @note This class has no out-of-line virtual (no "key function"), so its vtable would
     * otherwise need emitting wherever `new OtelLogBridge` is first written — fine for a
     * same-.so caller (e.g. the engine's `ServerRunner::load_plugins()`, itself compiled into
     * `congelado_sdk`), but a genuine link failure (`undefined symbol: vtable for
     * OtelLogBridge`) for a caller in a separate translation unit/executable like
     * `congelado_worker`'s `worker_main.cc`, which doesn't otherwise pull in a definition. This
     * static factory keeps every `new OtelLogBridge` confined to this one TU, mirroring exactly
     * how `LoggerAdapter::register_from()` above already keeps its own construction in-module.
     * @param registry the logger registry to register the new bridge into.
     */
    static void install(core::logger::LoggerRegistry &registry) {
        std::make_shared<OtelLogBridge>()->register_bridge(registry);
    }

  private:
    /**
     * @brief Maps `core::logger`'s six-level scheme onto OTel's five-level severity scheme —
     * `IMPORTANT` (a level `interfaces::LogSeverity` has no equivalent for) collapses to `INFO`.
     * @param level the logger-side level.
     * @return the closest OTel severity.
     */
    static interfaces::LogSeverity to_otel_severity(interfaces::LogLevel level) noexcept {
        switch (level) {
        case interfaces::LogLevel::DEBUG:
            return interfaces::LogSeverity::DEBUG;
        case interfaces::LogLevel::INFO:
        case interfaces::LogLevel::IMPORTANT:
            return interfaces::LogSeverity::INFO;
        case interfaces::LogLevel::WARNING:
            return interfaces::LogSeverity::WARN;
        case interfaces::LogLevel::ERROR:
            return interfaces::LogSeverity::ERROR;
        case interfaces::LogLevel::FATAL:
            return interfaces::LogSeverity::FATAL;
        }
        return interfaces::LogSeverity::INFO;
    }

    /**
     * @brief Builds a `LogRecord` (stamping it with the calling thread's ambient trace/span
     * context, if any) and fans it out to every registered `ILogRecordProvider`. Never throws —
     * degrades to a silent no-op if no `LogRecordRegistry` is active or nothing's registered.
     * @note Never forwards a DEBUG-severity line tagged `otel*` (e.g. `otel_traces`/
     * `otel_metrics`/`otel_logs`/`otel_sdk`/`otel_otlp` — every tag this project's own OTel plugin
     * logs under, see `plugins/otel_otlp/src/otel_otlp_plugin.cc`). `core::logger::debug(name,
     * ...)` bakes `name` into the message as a leading `"|name| "` (see `core_logger`'s `log()`),
     * so checking that prefix here catches it. Without this, an OTel exporter's own routine
     * "exporting N record(s)"/"sent N successfully" diagnostic — logged at DEBUG on every single
     * export — would get forwarded right back into the OTel logs pipeline it's reporting on,
     * queue as a new record, trigger another export, log itself again, forever — an unbounded
     * feedback loop, not just noise. Scoped to DEBUG specifically so it doesn't also swallow the
     * plugin's one-time IMPORTANT startup line or WARNING-level export-failure lines (both
     * useful, and neither one is a routine per-export message that could compound like this) —
     * every *other* tag, and every non-DEBUG severity of an `otel*` tag, still forwards normally.
     * @param severity the record's OTel severity.
     * @param msg the record's body text.
     */
    void emit(interfaces::LogSeverity severity, std::string_view msg) noexcept {
        try {
            if (severity == interfaces::LogSeverity::DEBUG && msg.starts_with("|otel")) {
                return;
            }
            auto *registry = core::otel::LogRecordRegistry::get_active();
            if (registry == nullptr || !registry->has_provider()) {
                return;
            }
            interfaces::LogRecord record{.body = msg, .severity = severity};
            if (auto ctx = core::otel::current_context()) {
                record.trace_id = ctx->trace_id;
                record.span_id = ctx->span_id;
            }
            for (const auto &provider : registry->get_providers()) {
                provider->emit(record);
            }
        } catch (...) {
            // Telemetry must never take the logger fan-out down with it.
        }
    }
};

/**
 * @brief Resolves a loaded plugin's BRIDGE-capability interface pointer via
 * `congelado_call(BRIDGE, GET, ...)`, if it has one.
 * @param ref the loaded plugin's symbol table.
 * @return a non-owning `shared_ptr<interfaces::IBridge>` (same lifetime idiom as
 * `resolve_serde_format`), or `nullptr` if the plugin doesn't export the BRIDGE capability.
 * @note The bridge self-identifies which runtime it implements via `IBridge::runtime_name()` —
 * the caller doesn't need to guess from the plugin's own display name; see
 * `core::plugin::SharedLibrary::broadcast_bridge()`, which reads it directly off the bridge.
 */
[[nodiscard]] inline std::shared_ptr<interfaces::IBridge> resolve_bridge(PluginRef &ref) {
    auto call_fn = resolve_call_fn(ref, 32U); // CONGELADO_CAP_BRIDGE
    if (call_fn == nullptr) {
        return nullptr;
    }
    auto result = call_fn(CONGELADO_RUN_BRIDGE, CONGELADO_ACTION_GET, nullptr, 0);
    if (result.type_index != CG_PTR || result.v_ptr == nullptr) {
        return nullptr;
    }
    auto *bridge = static_cast<interfaces::IBridge *>(result.v_ptr);
    return std::shared_ptr<interfaces::IBridge>(bridge, [](interfaces::IBridge *) {});
}

} // namespace congelado::heart
