// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/device.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/device.h"

export module cc_ice_extern_stream_executor_builder:device;

import std;

export namespace ice::builder {

class TF_DeviceOps
{
public:
    TF_DeviceOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_DeviceOps(const TF_DeviceOps&) = delete;
    TF_DeviceOps& operator=(const TF_DeviceOps&) = delete;

    static TF_DeviceOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_DeviceOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DeviceOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_DeviceOps*>(handle->plugin_data);
    }

    virtual ~TF_DeviceOps() = default;
    virtual void get_numa_node(int32_t* out_numa_node) noexcept = 0;
    virtual void get_memory_bandwidth(int64_t* out_bandwidth) noexcept = 0;
    virtual void get_gflops(double* out_gflops) noexcept = 0;
    virtual void get_hardware_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void get_device_vendor(const ice::sonic::String& out_vendor) noexcept = 0;
    virtual void get_pci_bus_id(const ice::sonic::String& out_pci_bus_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_device_properties(TF_DeviceProperties* out_properties) noexcept = 0;
    virtual void get_native_handle(void** out_handle) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_DeviceOps{
            .struct_size = TF_DEVICE_STRUCT_SIZE,
            .get_numa_node =
                [](TF_Device* device, int32_t* out_numa_node) noexcept
            {
                TF_DeviceOps::from_handle(device).get_numa_node(out_numa_node);
            },
            .get_memory_bandwidth =
                [](TF_Device* device, int64_t* out_bandwidth) noexcept
            {
                TF_DeviceOps::from_handle(device).get_memory_bandwidth(out_bandwidth);
            },
            .get_gflops =
                [](TF_Device* device, double* out_gflops) noexcept
            {
                TF_DeviceOps::from_handle(device).get_gflops(out_gflops);
            },
            .get_hardware_name =
                [](TF_Device* device, TF_String* out_name) noexcept
            {
                TF_DeviceOps::from_handle(device).get_hardware_name(
                    ice::sonic::String::wrap(out_name)
                );
            },
            .get_device_vendor =
                [](TF_Device* device, TF_String* out_vendor) noexcept
            {
                TF_DeviceOps::from_handle(device).get_device_vendor(
                    ice::sonic::String::wrap(out_vendor)
                );
            },
            .get_pci_bus_id =
                [](TF_Device* device, TF_String* out_pci_bus_id) noexcept
            {
                TF_DeviceOps::from_handle(device).get_pci_bus_id(
                    ice::sonic::String::wrap(out_pci_bus_id)
                );
            },
            .get_device_properties =
                [](TF_Device* device,
                   TF_DeviceProperties* out_properties,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_DeviceOps::from_handle(device).get_device_properties(out_properties);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_native_handle =
                [](TF_Device* device, void** out_handle) noexcept
            {
                TF_DeviceOps::from_handle(device).get_native_handle(out_handle);
            },

        };
    }

    const ::TF_DeviceOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Device& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_DeviceOps m_vtable;
    TF_Device m_handle;
};

} // namespace ice::builder
