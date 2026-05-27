export module core_plugin:handler;

import std;
import interfaces;
import core_ffi;
import :handle;

export namespace core::plugin {

// Returns the bridge as ILogger, or nullptr if it does not advertise Cap::LOGGER.
[[nodiscard]]
inline std::shared_ptr<interfaces::ILogger> make_logger(const PluginHandle &bridge) {
    if (!bridge || !bridge->has(Cap::LOGGER))
        return nullptr;
    return bridge;
}

// Returns the protocol implementation, or nullptr if the plugin has no Cap::PROTOCOL.
[[nodiscard]]
inline std::shared_ptr<interfaces::IProtocol> make_protocol(const PluginHandle &bridge) {
    if (!bridge || !bridge->has(Cap::PROTOCOL))
        return nullptr;
    return bridge->get_protocol();
}

} // namespace core::plugin
