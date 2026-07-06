module;

#include "congelado/abi.h"

export module congelado_plugin;

import std;
import interfaces;
export import core_plugin;
export namespace congelado {

class Plugin : public interfaces::ILogger {
  public:
    Plugin() noexcept = default;
    ~Plugin() override = default;
    Plugin(const Plugin &) = delete;
    Plugin &operator=(const Plugin &) = delete;
    Plugin(Plugin &&) = delete;
    Plugin &operator=(Plugin &&) = delete;

    [[nodiscard]] virtual std::string_view get_name() const noexcept override = 0;
    [[nodiscard]] virtual std::string_view get_version() const noexcept = 0;
    [[nodiscard]] virtual std::uint32_t capabilities() const noexcept { return 0; }
    [[nodiscard]] virtual std::string_view get_unique_type() const noexcept { return {}; }
    [[nodiscard]] virtual std::string_view get_type() const noexcept { return "plugin"; }
    [[nodiscard]] virtual std::span<const std::string_view> get_requires() const noexcept { return {}; }
    [[nodiscard]] virtual std::span<const std::string_view> get_load_before_types() const noexcept { return {}; }

    [[nodiscard]] virtual std::string_view get_worker_type() const noexcept { return {}; }
    [[nodiscard]] virtual CongeladoConfigView execute_worker(
        const CongeladoConfigView * /*input*/) {
        return {};
    }

    virtual void on_load(CongeladoHostCallbacks const & /*host*/,
                         CongeladoConfigView const & /*cfg*/) {}
    virtual void on_unload() noexcept {}
    virtual void on_ready() noexcept {}
    [[nodiscard]] virtual bool on_reload_requested() noexcept { return true; }

    virtual void logger_write(int /*level*/, std::string_view /*msg*/) noexcept {}
    void write(interfaces::LogLevel level, std::string_view msg) noexcept final {
        logger_write(static_cast<int>(level), msg);
    }
    void error(std::string_view msg) noexcept final { logger_write(4, msg); }
};

namespace _cap_dispatch {

template <typename T>
concept has_logger_write = requires(T *p) { p->logger_write(int{}, std::string_view{}); };

template <typename T>
void logger_write(T *p, int level, std::string_view msg) noexcept {
    if constexpr (has_logger_write<T>)
        p->logger_write(level, msg);
}

template <typename T>
concept has_protocol_get = requires(T *p) { p->protocol_get(); };

template <typename T>
void *protocol_get(T *p) noexcept {
    if constexpr (has_protocol_get<T>)
        return p->protocol_get();
    return nullptr;
}

template <typename T>
concept has_storage_get = requires(T *p) { p->storage_get(); };

template <typename T>
void *storage_get(T *p) noexcept {
    if constexpr (has_storage_get<T>)
        return p->storage_get();
    return nullptr;
}

} // namespace _cap_dispatch

// ── Re-exports ───────────────────────────────────────────────────────────────

using core::plugin::types::router_ctx;
using core::plugin::types::controller_ctx;
using core::plugin::types::leverager_ctx;
using core::plugin::types::config_get;
using core::plugin::types::config_for_each;
using core::plugin::types::ConfigViewBuilder;
using FfiRuntime = core::plugin::FfiRuntime;

using GenerationConfig = core::plugin::types::GenerationConfig;
using Runtime = core::plugin::types::Runtime;
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
