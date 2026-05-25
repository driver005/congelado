module;

#include <stdio.h>

export module core_plugin:handler;

import std;
import shared;
import interfaces;
import core_ffi;
import :handle;

export namespace core::plugin {

// FfiBridge IS the handler — it inherits shared::HandlerBase directly.
// This module exposes a helper to log errors from bridge callbacks
// and a factory to wrap a PluginHandle (shared_ptr<FfiBridge>) in an
// interface implementation for a given capability.

inline void log_bridge_error(std::string_view bridge_name, std::exception_ptr eptr) noexcept {
    try {
        std::rethrow_exception(eptr);
    } catch (const std::exception &ex) {
        std::println(stderr, "[ffi::{}] error: {}", bridge_name, ex.what());
    } catch (...) {
        std::println(stderr, "[ffi::{}] unknown error", bridge_name);
    }
}

// Returns the bridge as ILogger, or nullptr if it does not advertise Cap::Logger.
[[nodiscard]]
inline std::shared_ptr<interfaces::ILogger> make_logger(const PluginHandle &bridge) {
    if (!bridge || !bridge->has(Cap::Logger))
        return nullptr;
    return bridge;
}

// Returns the protocol implementation, or nullptr if the plugin has no Cap::Protocol.
[[nodiscard]]
inline std::shared_ptr<interfaces::IProtocol> make_protocol(const PluginHandle &bridge) {
    if (!bridge || !bridge->has(Cap::Protocol))
        return nullptr;
    return bridge->get_protocol();
}


} // namespace core::plugin
