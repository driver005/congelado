module core.manager.handler;
@nogc nothrow:

import interfaces.logger   : ILogger;
import core.ffi.bridge     : Cap;
import core.manager.handle : PluginHandle;

// Returns the bridge as ILogger, or null if it does not advertise Cap::LOGGER.
ILogger make_logger(PluginHandle bridge) {
    if (bridge is null || !bridge.has(Cap.LOGGER))
        return null;
    return bridge;
}

// // Returns the protocol implementation, or null if the plugin has no Cap::PROTOCOL.
// IProtocol make_protocol(PluginHandle bridge) {
//     if (bridge is null || !bridge.has(Cap.PROTOCOL))
//         return null;
//     return bridge.get_protocol();
// }
//
// // Returns the storage implementation, or null if the plugin has no Cap::STORAGE.
// IDatabase make_storage(PluginHandle bridge) {
//     if (bridge is null || !bridge.has(Cap.STORAGE))
//         return null;
//     return bridge.get_storage();
// }
