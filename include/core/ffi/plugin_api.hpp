#pragma once
#include "plugin_api.h"
#include <string_view>
#include <cstring>

namespace congelado {

/* C++ base class for plugin authors. Inherit this, override virtual methods,
   drop CONGELADO_PLUGIN(YourClass) at bottom of .cc — zero C required. */
class PluginBase {
public:
    PluginBase() noexcept = default;
    virtual ~PluginBase() = default;
    PluginBase(const PluginBase&) = delete;
    PluginBase& operator=(const PluginBase&) = delete;
    PluginBase(PluginBase&&) = delete;
    PluginBase& operator=(PluginBase&&) = delete;

    // IMPORTANT: returned string_view::data() is stored as a raw const char* in CongeladoPlugin.
    // Implementations MUST return a view into a string literal or a member with plugin lifetime.
    [[nodiscard]] virtual std::string_view name()    const noexcept = 0;
    [[nodiscard]] virtual std::string_view version() const noexcept = 0;
    virtual void on_load(const CongeladoHostCallbacks& /*host*/, const CongeladoConfigView* /*cfg*/) {}
    virtual void on_unload() {}

    /* Override to expose logger capability. Return non-null if this plugin is a logger. */
    virtual CongeladoLoggerCap*   logger_cap()   noexcept { return nullptr; }
    /* Override to expose protocol capability. Return non-null if this plugin is a protocol.
       For now returns interfaces::IProtocol* reinterpret_cast'd — placeholder. */
    virtual CongeladoProtocolCap* protocol_cap() noexcept { return nullptr; }

    /* Builds a CongeladoPlugin C-struct backed by this object.
       The returned struct holds a raw pointer to this — caller must keep this alive. */
    CongeladoPlugin to_c_plugin() noexcept {
        CongeladoPlugin plugin{};
        plugin.name    = name().data();
        plugin.version = version().data();
        plugin.self    = this;
        plugin.on_load = [](void* self, const CongeladoHostCallbacks* host, const CongeladoConfigView* cfg) {
            static_cast<PluginBase*>(self)->on_load(*host, cfg);
        };
        plugin.on_unload = [](void* self) {
            static_cast<PluginBase*>(self)->on_unload();
        };
        plugin.get_capability = [](void* self, uint32_t cap_id) -> void* {
            auto* pb = static_cast<PluginBase*>(self);
            switch (cap_id) {
            case CONGELADO_CAP_LOGGER:   return pb->logger_cap();
            case CONGELADO_CAP_PROTOCOL: return pb->protocol_cap();
            default:                     return nullptr;
            }
        };
        return plugin;
    }
};

} // namespace congelado

/* Drop exactly once at the bottom of your plugin .cc.
   Generates the two extern "C" symbols dlopen looks for. */
#define CONGELADO_PLUGIN(T) /* NOLINT(cppcoreguidelines-macro-usage) */        \
    extern "C" CongeladoPlugin* congelado_get_plugin() {                       \
        auto* plugin = new T{};                                                \
        auto* desc   = new CongeladoPlugin(plugin->to_c_plugin());             \
        return desc;                                                           \
    }                                                                          \
    extern "C" void congelado_destroy_plugin(CongeladoPlugin* desc) {          \
        delete static_cast<T*>(desc->self); /* NOLINT(bugprone-macro-parentheses) */ \
        delete desc;                        /* NOLINT(bugprone-macro-parentheses) */ \
    }
