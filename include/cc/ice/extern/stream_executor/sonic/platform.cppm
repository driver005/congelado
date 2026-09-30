// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/platform.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/platform.h"

export module cc_ice_extern_stream_executor_sonic:platform;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_PlatformOps : public ice::sonic::Runtime<TF_PlatformOps, TF_PlatformOps>
{
public:
    explicit TF_PlatformOps(TF_PlatformOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "stream_executor";

    [[nodiscard]] std::expected<void, ice::Status> get_device_count(int* out_device_count) noexcept
    {
        ice::Status status;
        m_ops->get_device_count(get_handle(), out_device_count, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    create_device_internal(const ice::sonic::TF_DeviceOps& device) noexcept
    {
        ice::Status status;
        m_ops->create_device_internal(get_handle(), device.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    destroy_device_internal(const ice::sonic::TF_DeviceOps& device) noexcept
    {
        ice::Status status;
        m_ops->destroy_device_internal(get_handle(), device.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    create_executor_internal(const ice::sonic::TF_ExecutorOps& executor) noexcept
    {
        ice::Status status;
        m_ops->create_executor_internal(get_handle(), executor.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    destroy_executor_internal(const ice::sonic::TF_ExecutorOps& executor) noexcept
    {
        ice::Status status;
        m_ops->destroy_executor_internal(get_handle(), executor.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_current_device(int* out_device_index) noexcept
    {
        ice::Status status;
        m_ops->get_current_device(get_handle(), out_device_index, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_current_device(int device_index) noexcept
    {
        ice::Status status;
        m_ops->set_current_device(get_handle(), device_index, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_device_for_pointer(const void* pointer, int* out_device_index) noexcept
    {
        ice::Status status;
        m_ops->get_device_for_pointer(get_handle(), pointer, out_device_index, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    can_access_peer(int device_index, int peer_device_index, _Bool* out_can_access) noexcept
    {
        ice::Status status;
        m_ops->can_access_peer(
            get_handle(),
            device_index,
            peer_device_index,
            out_can_access,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_native_handle(void** out_handle) noexcept
    {
        ice::Status status;
        m_ops->get_native_handle(get_handle(), out_handle, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    sonic::String get_name() const noexcept
    {
        sonic::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
