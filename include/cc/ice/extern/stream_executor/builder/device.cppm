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
    static TF_DeviceOps* create(void* ctx) noexcept
    {
        return static_cast<TF_DeviceOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DeviceOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_DeviceOps*>(handle->plugin_data);
    }

    virtual ~TF_DeviceOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_numa_node(int32_t* out_numa_node) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_memory_bandwidth(int64_t* out_bandwidth) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_gflops(double* out_gflops) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_hardware_name(const ice::sonic::TF_StringOps& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_device_vendor(const ice::sonic::TF_StringOps& out_vendor) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_pci_bus_id(const ice::sonic::TF_StringOps& out_pci_bus_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_device_properties(TF_DeviceProperties* out_properties) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_native_handle(void** out_handle) noexcept = 0;

    static TF_DeviceOps* get_generic_vtable()
    {
        static TF_DeviceOps vtable = {
            .struct_size = TF_DEVICE_STRUCT_SIZE,
            .get_numa_node =
                [](TF_Device* device, int32_t* out_numa_node) noexcept
            {
                auto* self = TF_DeviceOps::create(device);
                auto res = self->get_numa_node(out_numa_node);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_memory_bandwidth =
                [](TF_Device* device, int64_t* out_bandwidth) noexcept
            {
                auto* self = TF_DeviceOps::create(device);
                auto res = self->get_memory_bandwidth(out_bandwidth);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_gflops =
                [](TF_Device* device, double* out_gflops) noexcept
            {
                auto* self = TF_DeviceOps::create(device);
                auto res = self->get_gflops(out_gflops);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_hardware_name =
                [](TF_Device* device, TF_String* out_name) noexcept
            {
                auto* self = TF_DeviceOps::create(device);
                auto res = self->get_hardware_name(ice::sonic::TF_StringOps::wrap(out_name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_device_vendor =
                [](TF_Device* device, TF_String* out_vendor) noexcept
            {
                auto* self = TF_DeviceOps::create(device);
                auto res = self->get_device_vendor(ice::sonic::TF_StringOps::wrap(out_vendor));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_pci_bus_id =
                [](TF_Device* device, TF_String* out_pci_bus_id) noexcept
            {
                auto* self = TF_DeviceOps::create(device);
                auto res = self->get_pci_bus_id(ice::sonic::TF_StringOps::wrap(out_pci_bus_id));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_device_properties =
                [](TF_Device* device,
                   TF_DeviceProperties* out_properties,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_DeviceOps::create(device);
                auto res = self->get_device_properties(out_properties);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_native_handle =
                [](TF_Device* device, void** out_handle) noexcept
            {
                auto* self = TF_DeviceOps::create(device);
                auto res = self->get_native_handle(out_handle);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
