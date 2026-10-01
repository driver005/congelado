// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/registration/registration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_registration_sonic:registration;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_RegistrationOps : public ice::sonic::Runtime<::TF_RegistrationOps, ::TF_Registration>
{
public:
    template<typename Registry>
    TF_RegistrationOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_RegistrationOps(
        Registry& registry,
        ::TF_Registration* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_RegistrationOps(const ::TF_RegistrationOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_RegistrationOps(const ::TF_RegistrationOps* ops, ::TF_Registration* handle) noexcept :
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

    void register_op(
        const ice::sonic::String& type,
        const ice::sonic::String& name,
        void* value
    ) const noexcept
    {
        m_ops->register_op(get_handle(), type.get_handle(), name.get_handle(), value);
    }

    void get(
        const ice::sonic::String& type,
        const ice::sonic::String& name,
        void** out_value
    ) const noexcept
    {
        m_ops->get(get_handle(), type.get_handle(), name.get_handle(), out_value);
    }

    void unregister(const ice::sonic::String& type, const ice::sonic::String& name) const noexcept
    {
        m_ops->unregister(get_handle(), type.get_handle(), name.get_handle());
    }

    void set_default(const ice::sonic::String& type, const ice::sonic::String& name) const noexcept
    {
        m_ops->set_default(get_handle(), type.get_handle(), name.get_handle());
    }
};

} // namespace ice::sonic
