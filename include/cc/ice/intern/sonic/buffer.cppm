// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/buffer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/buffer.h"

export module cc_ice_intern_sonic:buffer;

import std;
import :runtime;
import :tstring;

export namespace ice::sonic {

class TF_BufferOps : public ice::sonic::Runtime<::TF_BufferOps, ::TF_Buffer>
{
public:
    TF_BufferOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_BufferOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Buffer* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_BufferOps(const ::TF_BufferOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_BufferOps(const ::TF_BufferOps* ops, ::TF_Buffer* handle) noexcept :
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

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void assign_from_string(const void* proto, size_t proto_len) const noexcept
    {
        m_ops->assign_from_string(get_handle(), proto, proto_len);
    }

    void delete_buffer() const noexcept
    {
        m_ops->delete_buffer(get_handle());
    }

    void get_buffer(TFBufferData* out_buffer) const noexcept
    {
        m_ops->get_buffer(get_handle(), out_buffer);
    }
};

} // namespace ice::sonic
