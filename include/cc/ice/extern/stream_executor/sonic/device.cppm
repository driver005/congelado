// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/device.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/device.h"

export module cc_ice_extern_stream_executor_sonic:device;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_DeviceOps : public ice::sonic::Runtime<::TF_DeviceOps, ::TF_Device>
{
public:
    TF_DeviceOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_DeviceOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Device* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_DeviceOps(const ::TF_DeviceOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_DeviceOps(const ::TF_DeviceOps* ops, ::TF_Device* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_numa_node(int32_t* out_numa_node) const noexcept
    {
        m_ops->get_numa_node(get_handle(), out_numa_node);
    }

    void get_memory_bandwidth(int64_t* out_bandwidth) const noexcept
    {
        m_ops->get_memory_bandwidth(get_handle(), out_bandwidth);
    }

    void get_gflops(double* out_gflops) const noexcept
    {
        m_ops->get_gflops(get_handle(), out_gflops);
    }

    void get_hardware_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_hardware_name(get_handle(), out_name.get_handle());
    }

    void get_device_vendor(const ice::sonic::String& out_vendor) const noexcept
    {
        m_ops->get_device_vendor(get_handle(), out_vendor.get_handle());
    }

    void get_pci_bus_id(const ice::sonic::String& out_pci_bus_id) const noexcept
    {
        m_ops->get_pci_bus_id(get_handle(), out_pci_bus_id.get_handle());
    }

    void get_device_properties(
        TF_DeviceProperties* out_properties,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_device_properties(get_handle(), out_properties, out_status.get_handle());
    }

    void get_native_handle(void** out_handle) const noexcept
    {
        m_ops->get_native_handle(get_handle(), out_handle);
    }
};

} // namespace ice::sonic
