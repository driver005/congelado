// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/platform.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/platform.h"

export module cc_ice_extern_stream_executor_builder:platform;

import std;

export namespace ice::builder {

class TF_PlatformOps
{
public:
    static TF_PlatformOps* create(void* ctx) noexcept
    {
        return static_cast<TF_PlatformOps*>(ctx);
    }

    template<typename HandleT>
    static TF_PlatformOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_PlatformOps*>(handle->plugin_data);
    }

    virtual ~TF_PlatformOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_device_count(int* out_device_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    create_device_internal(const ice::sonic::TF_DeviceOps& device) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    destroy_device_internal(const ice::sonic::TF_DeviceOps& device) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    create_executor_internal(const ice::sonic::TF_ExecutorOps& executor) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    destroy_executor_internal(const ice::sonic::TF_ExecutorOps& executor) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_current_device(int* out_device_index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_current_device(int device_index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_device_for_pointer(const void* pointer, int* out_device_index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    can_access_peer(int device_index, int peer_device_index, _Bool* out_can_access) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_native_handle(void** out_handle) noexcept = 0;

    static TF_PlatformOps* get_generic_vtable()
    {
        static TF_PlatformOps vtable = {
            .struct_size = TF_PLATFORM_STRUCT_SIZE,
            .get_device_count =
                [](TF_Platform* platform, int* out_device_count, TF_Status* out_status) noexcept
            {
                auto* self = TF_PlatformOps::create(platform);
                auto res = self->get_device_count(out_device_count);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .create_device_internal =
                [](TF_Platform* platform, TF_Device* device, TF_Status* out_status) noexcept
            {
                auto* self = TF_PlatformOps::create(platform);
                auto res = self->create_device_internal(ice::sonic::TF_DeviceOps::wrap(device));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_device_internal =
                [](TF_Platform* platform, TF_Device* device) noexcept
            {
                auto* self = TF_PlatformOps::create(platform);
                auto res = self->destroy_device_internal(ice::sonic::TF_DeviceOps::wrap(device));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_executor_internal =
                [](TF_Platform* platform, TF_Executor* executor, TF_Status* out_status) noexcept
            {
                auto* self = TF_PlatformOps::create(platform);
                auto res =
                    self->create_executor_internal(ice::sonic::TF_ExecutorOps::wrap(executor));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_executor_internal =
                [](TF_Platform* platform, TF_Executor* executor) noexcept
            {
                auto* self = TF_PlatformOps::create(platform);
                auto res =
                    self->destroy_executor_internal(ice::sonic::TF_ExecutorOps::wrap(executor));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_current_device =
                [](TF_Platform* platform, int* out_device_index, TF_Status* out_status) noexcept
            {
                auto* self = TF_PlatformOps::create(platform);
                auto res = self->get_current_device(out_device_index);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_current_device =
                [](TF_Platform* platform, int device_index, TF_Status* out_status) noexcept
            {
                auto* self = TF_PlatformOps::create(platform);
                auto res = self->set_current_device(device_index);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_device_for_pointer =
                [](TF_Platform* platform,
                   const void* pointer,
                   int* out_device_index,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_PlatformOps::create(platform);
                auto res = self->get_device_for_pointer(pointer, out_device_index);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .can_access_peer =
                [](TF_Platform* platform,
                   int device_index,
                   int peer_device_index,
                   _Bool* out_can_access,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_PlatformOps::create(platform);
                auto res = self->can_access_peer(device_index, peer_device_index, out_can_access);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_native_handle =
                [](TF_Platform* platform, void** out_handle) noexcept
            {
                auto* self = TF_PlatformOps::create(platform);
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
