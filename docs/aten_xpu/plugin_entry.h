#ifndef ATEN_XPU_PLUGIN_ENTRY_H_
#define ATEN_XPU_PLUGIN_ENTRY_H_

// Entry points shared by every plugin library of the aten_xpu reference. A library defines
// ATEN_XPU_DOMAIN (a SyclPluginDomain bit mask) before using these, imports aten_xpu, and exports
// only the create_* functions of its own domain; the registry brings up only that domain.

#define ATEN_XPU_EXPORT_OPS(name, ops_type, accessor)                                              \
    TF_CAPI_EXPORT void create_##name(                                                             \
        ops_type** ops,                                                                            \
        void** plugin_context,                                                                     \
        TF_Status* out_status                                                                      \
    )                                                                                              \
    {                                                                                              \
        static_cast<void>(out_status);                                                             \
        auto& registry = aten_xpu::SyclPluginRegistry::getInstance();                              \
        registry.initialize(ATEN_XPU_DOMAIN);                                                      \
        *ops = const_cast<ops_type*>(accessor);                                                    \
        *plugin_context = &registry;                                                               \
    }                                                                                              \
    TF_CAPI_EXPORT void destroy_##name(void* plugin_context)                                       \
    {                                                                                              \
        static_cast<void>(plugin_context);                                                         \
    }

// Status, String and Buffer are the host's primitives: weak, so a real host definition wins, and no
// plugin initialisation (the plugin itself calls these while loading the host ops).
#define ATEN_XPU_EXPORT_WEAK_OPS(name, ops_type, accessor)                                         \
    __attribute__((weak)) TF_CAPI_EXPORT void create_##name(                                       \
        ops_type** ops,                                                                            \
        void** plugin_context,                                                                     \
        TF_Status* out_status                                                                      \
    )                                                                                              \
    {                                                                                              \
        static_cast<void>(out_status);                                                             \
        auto& registry = aten_xpu::SyclPluginRegistry::getInstance();                              \
        *ops = const_cast<ops_type*>(accessor);                                                    \
        *plugin_context = &registry;                                                               \
    }                                                                                              \
    __attribute__((weak)) TF_CAPI_EXPORT void destroy_##name(void* plugin_context)                 \
    {                                                                                              \
        static_cast<void>(plugin_context);                                                         \
    }


#define ATEN_XPU_PLUGIN_ENTRY(plugin_name)                                                         \
    TF_CAPI_EXPORT void create_plugin(TF_PluginInfo* plugin_info)                                  \
    {                                                                                              \
        auto& registry = aten_xpu::SyclPluginRegistry::getInstance();                              \
        registry.initialize(ATEN_XPU_DOMAIN);                                                      \
        plugin_info->name = plugin_name;                                                           \
        plugin_info->version = aten_xpu::k_plugin_version.data();                                  \
    }

#endif // ATEN_XPU_PLUGIN_ENTRY_H_
