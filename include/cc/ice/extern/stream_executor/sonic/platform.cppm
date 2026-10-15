// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/platform.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/platform.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_stream_executor_sonic:platform;

import std;
import :device;
import :executor;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class Platform : public ice::sonic::Runtime<::TF_PlatformOps, ::TF_Platform>
{
public:
    Platform(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    Platform(
        const ice::sonic::Registration& registry,
        ::TF_Platform* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            handle,
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    explicit Platform(const ::TF_PlatformOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Platform(const ::TF_PlatformOps* ops, ::TF_Platform* handle) noexcept :
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

    void get_device_count(
        int& out_device_count,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_device_count(get_handle(), &out_device_count, out_status.get_handle());
    }

    void create_device_internal(
        const ice::sonic::Device& device,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_device_internal(get_handle(), device.get_handle(), out_status.get_handle());
    }

    void destroy_device_internal(const ice::sonic::Device& device) const noexcept
    {
        m_ops->destroy_device_internal(get_handle(), device.get_handle());
    }

    void create_executor_internal(
        const ice::sonic::Executor& executor,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_executor_internal(
            get_handle(),
            executor.get_handle(),
            out_status.get_handle()
        );
    }

    void destroy_executor_internal(const ice::sonic::Executor& executor) const noexcept
    {
        m_ops->destroy_executor_internal(get_handle(), executor.get_handle());
    }

    void get_current_device(
        int& out_device_index,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_current_device(get_handle(), &out_device_index, out_status.get_handle());
    }

    void set_current_device(int device_index, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_current_device(get_handle(), device_index, out_status.get_handle());
    }

    void get_device_for_pointer(
        const void* pointer,
        int& out_device_index,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_device_for_pointer(
            get_handle(),
            pointer,
            &out_device_index,
            out_status.get_handle()
        );
    }

    void can_access_peer(
        int device_index,
        int peer_device_index,
        bool& out_can_access,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->can_access_peer(
            get_handle(),
            device_index,
            peer_device_index,
            &out_can_access,
            out_status.get_handle()
        );
    }

    void get_native_handle(void** out_handle) const noexcept
    {
        m_ops->get_native_handle(get_handle(), out_handle);
    }
};

} // namespace ice::sonic
