// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tstring.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/tstring.h"

export module cc_ice_intern_sonic:tstring;

import std;
import :runtime;

export namespace ice::sonic {

class String : public ice::sonic::Runtime<::TF_StringOps, ::TF_String>
{
public:
    String(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    String(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_String* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit String(const ::TF_StringOps* ops) noexcept :
        Runtime(ops)
    {
    }

    String(const ::TF_StringOps* ops, ::TF_String* handle) noexcept :
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

    void init() const noexcept
    {
        m_ops->init(get_handle());
    }

    void copy(const char* src, size_t size) const noexcept
    {
        m_ops->copy(get_handle(), src, size);
    }

    void assign_view(const char* src, size_t size) const noexcept
    {
        m_ops->assign_view(get_handle(), src, size);
    }

    void get_data_pointer(const char** out_data) const noexcept
    {
        m_ops->get_data_pointer(get_handle(), out_data);
    }

    void get_type(TFTStringType* out_type) const noexcept
    {
        m_ops->get_type(get_handle(), out_type);
    }

    void get_size(size_t* out_size) const noexcept
    {
        m_ops->get_size(get_handle(), out_size);
    }

    void get_capacity(size_t* out_capacity) const noexcept
    {
        m_ops->get_capacity(get_handle(), out_capacity);
    }

    void dealloc() const noexcept
    {
        m_ops->dealloc(get_handle());
    }
};

} // namespace ice::sonic
