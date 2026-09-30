// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/device.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/device.h"

export module cc_ice_extern_stream_executor_sonic:device;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_DeviceOps : public ice::sonic::Runtime<TF_DeviceOps, TF_DeviceOps>
{
public:
    explicit TF_DeviceOps(TF_DeviceOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "stream_executor";

    void get_numa_node(int32_t* out_numa_node) noexcept
    {
        m_ops->get_numa_node(get_handle(), out_numa_node);
    }

    void get_memory_bandwidth(int64_t* out_bandwidth) noexcept
    {
        m_ops->get_memory_bandwidth(get_handle(), out_bandwidth);
    }

    void get_gflops(double* out_gflops) noexcept
    {
        m_ops->get_gflops(get_handle(), out_gflops);
    }

    void get_hardware_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_hardware_name(get_handle(), out_name.get_handle());
    }

    void get_device_vendor(const ice::sonic::String& out_vendor) noexcept
    {
        m_ops->get_device_vendor(get_handle(), out_vendor.get_handle());
    }

    void get_pci_bus_id(const ice::sonic::String& out_pci_bus_id) noexcept
    {
        m_ops->get_pci_bus_id(get_handle(), out_pci_bus_id.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_device_properties(TF_DeviceProperties* out_properties) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_device_properties(get_handle(), out_properties, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void get_native_handle(void** out_handle) noexcept
    {
        m_ops->get_native_handle(get_handle(), out_handle);
    }
};

} // namespace ice::sonic
