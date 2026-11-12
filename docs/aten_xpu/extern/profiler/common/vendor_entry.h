#ifndef ATEN_XPU_EXTERN_PROFILER_COMMON_VENDOR_ENTRY_H_
#define ATEN_XPU_EXTERN_PROFILER_COMMON_VENDOR_ENTRY_H_

// The C entry points of a profiler plugin library: create_profiler / destroy_profiler for the
// profiler and create_tensor / destroy_tensor for the host-memory tensor it returns. The caller
// must already import aten_xpu_profiler_common and include the profiler and tensor C headers.
#define ATEN_XPU_EXPORT_VENDOR_PROFILER(BackendType, DeviceType)                                   \
    namespace {                                                                                    \
    aten_xpu::VendorProfilerPlugin& vendor_plugin()                                                \
    {                                                                                              \
        static aten_xpu::VendorProfilerPlugin plugin{BackendType::instance(), DeviceType};         \
        return plugin;                                                                             \
    }                                                                                              \
    }                                                                                              \
    extern "C"                                                                                     \
    {                                                                                              \
        TF_CAPI_EXPORT void                                                                        \
        create_profiler(TF_ProfilerOps** ops, void** plugin_context, TF_Status* out_status)        \
        {                                                                                          \
            static_cast<void>(out_status);                                                         \
            auto& plugin = vendor_plugin();                                                        \
            plugin.initialize();                                                                   \
            *ops = const_cast<TF_ProfilerOps*>(&plugin.getProfilerOps());                          \
            *plugin_context = &plugin;                                                             \
        }                                                                                          \
        TF_CAPI_EXPORT void destroy_profiler(void* plugin_context)                                 \
        {                                                                                          \
            static_cast<void>(plugin_context);                                                     \
        }                                                                                          \
        TF_CAPI_EXPORT void                                                                        \
        create_tensor(TF_TensorOps** ops, void** plugin_context, TF_Status* out_status)            \
        {                                                                                          \
            static_cast<void>(out_status);                                                         \
            auto& plugin = vendor_plugin();                                                        \
            plugin.initialize();                                                                   \
            *ops = const_cast<TF_TensorOps*>(&plugin.getTensorOps());                              \
            *plugin_context = &plugin;                                                             \
        }                                                                                          \
        TF_CAPI_EXPORT void destroy_tensor(void* plugin_context)                                   \
        {                                                                                          \
            static_cast<void>(plugin_context);                                                     \
        }                                                                                          \
    }

#endif // ATEN_XPU_EXTERN_PROFILER_COMMON_VENDOR_ENTRY_H_
