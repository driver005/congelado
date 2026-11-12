module;

#include "include/c/extern/profiler/profiler.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_profiler_common:vendor_plugin;

import std;
import :backend;
import :host_ops;
import :host_tensor;
import :vendor_profiler;

export namespace aten_xpu {

// Everything one profiler plugin library owns: the host ops it loaded, and the vtables of its
// profiler and of the host-memory tensor it returns. One instance per library.
class VendorProfilerPlugin
{
public:
    VendorProfilerPlugin(ProfileBackend& backend, std::string device_type) :
        m_backend{backend},
        m_device_type{std::move(device_type)},
        m_profiler{m_ops, m_backend, m_device_type},
        m_tensor{m_ops}
    {
    }

    VendorProfilerPlugin(const VendorProfilerPlugin&) = delete;
    VendorProfilerPlugin& operator=(const VendorProfilerPlugin&) = delete;
    VendorProfilerPlugin(VendorProfilerPlugin&&) = delete;
    VendorProfilerPlugin& operator=(VendorProfilerPlugin&&) = delete;
    ~VendorProfilerPlugin() = default;

    void initialize()
    {
        if (m_initialized) {
            return;
        }
        m_initialized = true;
        s_active = this;

        ::TF_StatusOps* status_ops = nullptr;
        ::TF_StringOps* string_ops = nullptr;
        create_status(&status_ops, &m_host_contexts[0], nullptr);
        create_string(&string_ops, &m_host_contexts[1], nullptr);
        m_ops.setStatusOps(status_ops);
        m_ops.setStringOps(string_ops);

        m_profiler.get_generic_vtable(
            [](::TF_Profiler* handle)
            {
                auto* object = new VendorProfiler{
                    s_active->m_ops,
                    s_active->m_backend,
                    s_active->m_device_type
                };
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_tensor.get_generic_vtable(
            [](::TF_Tensor* handle)
            {
                auto* object = new HostTensor{s_active->m_ops};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_ops.setTensorOps(&m_tensor.get_vtable());
    }

    [[nodiscard]] const ::TF_ProfilerOps& getProfilerOps() const noexcept
    {
        return m_profiler.get_vtable();
    }

    [[nodiscard]] const ::TF_TensorOps& getTensorOps() const noexcept
    {
        return m_tensor.get_vtable();
    }

private:
    inline static VendorProfilerPlugin* s_active = nullptr;

    ProfileBackend& m_backend;
    std::string m_device_type;
    ProfilerHostOps m_ops;
    std::array<void*, 2> m_host_contexts{};
    bool m_initialized{false};
    VendorProfiler m_profiler;
    HostTensor m_tensor;
};

} // namespace aten_xpu
