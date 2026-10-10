// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/platform.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/extern/stream_executor/platform.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_stream_executor_builder:platform;

import std;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_PlatformOps
{
public:
    explicit TF_PlatformOps(
        const ::TF_DeviceOps* TF_DeviceOps_ops,
        const ::TF_ExecutorOps* TF_ExecutorOps_ops,
        const ::TF_StatusOps* Status_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_DeviceOps_ops = TF_DeviceOps_ops;
        m_TF_ExecutorOps_ops = TF_ExecutorOps_ops;
        m_Status_ops = Status_ops;
    }

    TF_PlatformOps(const TF_PlatformOps&) = delete;
    TF_PlatformOps& operator=(const TF_PlatformOps&) = delete;

    static TF_PlatformOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_PlatformOps*>(ctx);
    }

    template<typename HandleT>
    static TF_PlatformOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_PlatformOps*>(handle->plugin_data);
    }

    virtual ~TF_PlatformOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void
    get_device_count(int* out_device_count, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void create_device_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_device_internal(const ice::sonic::TF_DeviceOps& device) noexcept = 0;
    virtual void create_executor_internal(
        const ice::sonic::TF_ExecutorOps& executor,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_executor_internal(const ice::sonic::TF_ExecutorOps& executor) noexcept = 0;
    virtual void
    get_current_device(int* out_device_index, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    set_current_device(int device_index, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_device_for_pointer(
        const void* pointer,
        int* out_device_index,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void can_access_peer(
        int device_index,
        int peer_device_index,
        _Bool* out_can_access,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_native_handle(void** out_handle) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Platform*)) noexcept
    {
        m_vtable = ::TF_PlatformOps{
            .struct_size = TF_OFFSET_OF_END(::TF_PlatformOps, get_native_handle),

            .create = create,
            .destroy =
                [](TF_Platform* handle) noexcept
            {
                auto& self = TF_PlatformOps::from_handle(handle);
                self.destroy();
            },
            .get_device_count =
                [](TF_Platform* platform, int* out_device_count, TF_Status* out_status) noexcept
            {
                auto& self = TF_PlatformOps::from_handle(platform);
                self.get_device_count(
                    out_device_count,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_device_internal =
                [](TF_Platform* platform, TF_Device* device, TF_Status* out_status) noexcept
            {
                auto& self = TF_PlatformOps::from_handle(platform);
                self.create_device_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_device_internal =
                [](TF_Platform* platform, TF_Device* device) noexcept
            {
                auto& self = TF_PlatformOps::from_handle(platform);
                self.destroy_device_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device)
                );
            },
            .create_executor_internal =
                [](TF_Platform* platform, TF_Executor* executor, TF_Status* out_status) noexcept
            {
                auto& self = TF_PlatformOps::from_handle(platform);
                self.create_executor_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_ExecutorOps>{}, executor),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_executor_internal =
                [](TF_Platform* platform, TF_Executor* executor) noexcept
            {
                auto& self = TF_PlatformOps::from_handle(platform);
                self.destroy_executor_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_ExecutorOps>{}, executor)
                );
            },
            .get_current_device =
                [](TF_Platform* platform, int* out_device_index, TF_Status* out_status) noexcept
            {
                auto& self = TF_PlatformOps::from_handle(platform);
                self.get_current_device(
                    out_device_index,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_current_device =
                [](TF_Platform* platform, int device_index, TF_Status* out_status) noexcept
            {
                auto& self = TF_PlatformOps::from_handle(platform);
                self.set_current_device(
                    device_index,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_device_for_pointer =
                [](TF_Platform* platform,
                   const void* pointer,
                   int* out_device_index,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_PlatformOps::from_handle(platform);
                self.get_device_for_pointer(
                    pointer,
                    out_device_index,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .can_access_peer =
                [](TF_Platform* platform,
                   int device_index,
                   int peer_device_index,
                   _Bool* out_can_access,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_PlatformOps::from_handle(platform);
                self.can_access_peer(
                    device_index,
                    peer_device_index,
                    out_can_access,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_native_handle =
                [](TF_Platform* platform, void** out_handle) noexcept
            {
                auto& self = TF_PlatformOps::from_handle(platform);
                self.get_native_handle(out_handle);
            },

        };
    }

    ice::sonic::TF_DeviceOps
    wrap(std::type_identity<ice::sonic::TF_DeviceOps>, const ::TF_Device* handle) const noexcept
    {
        return ice::sonic::TF_DeviceOps{m_TF_DeviceOps_ops, const_cast<::TF_Device*>(handle)};
    }

    ice::sonic::TF_ExecutorOps
    wrap(std::type_identity<ice::sonic::TF_ExecutorOps>, const ::TF_Executor* handle) const noexcept
    {
        return ice::sonic::TF_ExecutorOps{m_TF_ExecutorOps_ops, const_cast<::TF_Executor*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_PlatformOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Platform& get_handle() const noexcept
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
            const_cast<::TF_PlatformOps*>(&m_vtable)
        );
    }

private:
    ::TF_PlatformOps m_vtable;
    ::TF_Platform m_handle;

    const ::TF_DeviceOps* m_TF_DeviceOps_ops{nullptr};

    const ::TF_ExecutorOps* m_TF_ExecutorOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
