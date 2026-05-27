#pragma once
#include "plugin_api.h"

#include <string_view>

namespace congelado {

/* C++ base class for plugin authors. Inherit this, override virtual methods,
   drop CONGELADO_PLUGIN(YourClass) at the bottom of your .cc — zero C required.

   Override capabilities() to advertise which capabilities your plugin provides.
   Override the matching methods for each advertised capability. */
class PluginBase {
  public:
    PluginBase() noexcept = default;
    virtual ~PluginBase() = default;
    PluginBase(const PluginBase &) = delete;
    PluginBase &operator=(const PluginBase &) = delete;
    PluginBase(PluginBase &&) = delete;
    PluginBase &operator=(PluginBase &&) = delete;

    // IMPORTANT: returned string_view::data() is used as a raw const char*.
    // Implementations MUST return a view into a string literal or stable member.
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual std::string_view version() const noexcept = 0;

    // Return bitmask of CONGELADO_CAP_* values for capabilities this plugin provides.
    [[nodiscard]] virtual uint32_t capabilities() const noexcept { return 0; }

    virtual void on_load(const CongeladoHostCallbacks & /*host*/, const CongeladoConfigView * /*cfg*/) {}
    virtual void on_unload() {}

    // Logger capability — override when capabilities() includes CONGELADO_CAP_LOGGER.
    virtual void logger_write(int /*level*/, const char * /*msg*/, size_t /*len*/) noexcept {}
    virtual void logger_write_error(const char * /*msg*/, size_t /*len*/) noexcept {}

    // Protocol capability — override when capabilities() includes CONGELADO_CAP_PROTOCOL.
    // Return IProtocol* cast to void*. Returned pointer must outlive the plugin.
    virtual void *protocol_get() noexcept { return nullptr; }
};

} // namespace congelado

/* Drop exactly once at the bottom of your plugin .cc.
   Generates all well-known C symbols the bridge discovers via dlsym. */
#define CONGELADO_PLUGIN(T)       /* NOLINT(cppcoreguidelines-macro-usage) */                                          \
    static T *s_plugin = nullptr; /* NOLINT(cppcoreguidelines-avoid-non-const-global-variables) */                     \
    extern "C" const char *congelado_plugin_name() noexcept {                                                          \
        if (s_plugin == nullptr)                                                                                       \
            s_plugin = new T{};                                                                                        \
        return s_plugin->name().data();                                                                                \
    }                                                                                                                  \
    extern "C" const char *congelado_plugin_version() noexcept {                                                       \
        if (s_plugin == nullptr)                                                                                       \
            s_plugin = new T{};                                                                                        \
        return s_plugin->version().data();                                                                             \
    }                                                                                                                  \
    extern "C" uint32_t congelado_capabilities() noexcept {                                                            \
        if (s_plugin == nullptr)                                                                                       \
            s_plugin = new T{};                                                                                        \
        return s_plugin->capabilities();                                                                               \
    }                                                                                                                  \
    extern "C" void congelado_on_load(const CongeladoHostCallbacks *host, const CongeladoConfigView *cfg) {            \
        if (s_plugin == nullptr)                                                                                       \
            s_plugin = new T{};                                                                                        \
        s_plugin->on_load(*host, cfg);                                                                                 \
    }                                                                                                                  \
    extern "C" void congelado_on_unload() noexcept {                                                                   \
        if (s_plugin != nullptr) {                                                                                     \
            s_plugin->on_unload();                                                                                     \
            delete s_plugin; /* NOLINT(cppcoreguidelines-owning-memory) */                                             \
            s_plugin = nullptr;                                                                                        \
        }                                                                                                              \
    }                                                                                                                  \
    extern "C" void congelado_logger_write(int level, const char *msg, size_t len) noexcept {                          \
        if (s_plugin != nullptr)                                                                                       \
            s_plugin->logger_write(level, msg, len);                                                                   \
    }                                                                                                                  \
    extern "C" void congelado_logger_write_error(const char *msg, size_t len) noexcept {                               \
        if (s_plugin != nullptr)                                                                                       \
            s_plugin->logger_write_error(msg, len);                                                                    \
    }                                                                                                                  \
    extern "C" void *congelado_protocol_get() noexcept {                                                               \
        return s_plugin != nullptr ? s_plugin->protocol_get() : nullptr;                                               \
    }
