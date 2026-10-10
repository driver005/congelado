// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/device.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_stream_executor_builder:device;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_DeviceOps
{
public:
    explicit TF_DeviceOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void get_numa_node(int32_t* out_numa_node) noexcept = 0;
    virtual void get_memory_bandwidth(int64_t* out_bandwidth) noexcept = 0;
    virtual void get_gflops(double* out_gflops) noexcept = 0;
    virtual void get_hardware_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void get_device_vendor(const ice::sonic::String& out_vendor) noexcept = 0;
    virtual void get_pci_bus_id(const ice::sonic::String& out_pci_bus_id) noexcept = 0;
    virtual void get_device_properties(
        TF_DeviceProperties* out_properties,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_native_handle(void** out_handle) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Device*)) noexcept
    {
        m_vtable = ::TF_DeviceOps{
            .struct_size = TF_OFFSET_OF_END(::TF_DeviceOps, get_native_handle),

            .create = create,
            .destroy =
                [](TF_Device* handle) noexcept
            {
                auto& self = TF_DeviceOps::from_handle(handle);
                self.destroy();
            },
            .get_numa_node =
                [](TF_Device* device, int32_t* out_numa_node) noexcept
            {
                auto& self = TF_DeviceOps::from_handle(device);
                self.get_numa_node(out_numa_node);
            },
            .get_memory_bandwidth =
                [](TF_Device* device, int64_t* out_bandwidth) noexcept
            {
                auto& self = TF_DeviceOps::from_handle(device);
                self.get_memory_bandwidth(out_bandwidth);
            },
            .get_gflops =
                [](TF_Device* device, double* out_gflops) noexcept
            {
                auto& self = TF_DeviceOps::from_handle(device);
                self.get_gflops(out_gflops);
            },
            .get_hardware_name =
                [](TF_Device* device, TF_String* out_name) noexcept
            {
                auto& self = TF_DeviceOps::from_handle(device);
                self.get_hardware_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name)
                );
            },
            .get_device_vendor =
                [](TF_Device* device, TF_String* out_vendor) noexcept
            {
                auto& self = TF_DeviceOps::from_handle(device);
                self.get_device_vendor(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_vendor)
                );
            },
            .get_pci_bus_id =
                [](TF_Device* device, TF_String* out_pci_bus_id) noexcept
            {
                auto& self = TF_DeviceOps::from_handle(device);
                self.get_pci_bus_id(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_pci_bus_id)
                );
            },
            .get_device_properties =
                [](TF_Device* device,
                   TF_DeviceProperties* out_properties,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_DeviceOps::from_handle(device);
                self.get_device_properties(
                    out_properties,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_native_handle =
                [](TF_Device* device, void** out_handle) noexcept
            {
                auto& self = TF_DeviceOps::from_handle(device);
                self.get_native_handle(out_handle);
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_DeviceOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Device& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_DeviceOps*>(&m_vtable)
        );
    }

private:
    ::TF_DeviceOps m_vtable;
    ::TF_Device m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
