// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/stream.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/stream.h"

export module cc_ice_extern_stream_executor_sonic:stream;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_StreamOps : public ice::sonic::Runtime<::TF_StreamOps, ::TF_Stream>
{
public:
    TF_StreamOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_StreamOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Stream* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_StreamOps(const ::TF_StreamOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_StreamOps(const ::TF_StreamOps* ops, ::TF_Stream* handle) noexcept :
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

    void get_priority(int32_t* out_priority) const noexcept
    {
        m_ops->get_priority(get_handle(), out_priority);
    }

    void get_device_index(int* out_device_index) const noexcept
    {
        m_ops->get_device_index(get_handle(), out_device_index);
    }

    void query(_Bool* out_idle, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->query(get_handle(), out_idle, out_status.get_handle());
    }

    void synchronize(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->synchronize(get_handle(), out_status.get_handle());
    }

    void get_capture_status(
        TF_CaptureStatus* out_capture_status,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_capture_status(get_handle(), out_capture_status, out_status.get_handle());
    }

    void get_native_handle(void** out_handle) const noexcept
    {
        m_ops->get_native_handle(get_handle(), out_handle);
    }
};

} // namespace ice::sonic
