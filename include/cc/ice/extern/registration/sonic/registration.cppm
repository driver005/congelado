// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/registration/registration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_registration_sonic:registration;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_RegistrationOps : public ice::sonic::Runtime<TF_RegistrationOps, TF_RegistrationOps>
{
public:
    explicit TF_RegistrationOps(TF_RegistrationOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "registration";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void register_op(
        const ice::sonic::String& type,
        const ice::sonic::String& name,
        void* value
    ) noexcept
    {
        m_ops->register_op(get_handle(), type.get_handle(), name.get_handle(), value);
    }

    void get(
        const ice::sonic::String& type,
        const ice::sonic::String& name,
        void** out_value
    ) noexcept
    {
        m_ops->get(get_handle(), type.get_handle(), name.get_handle(), out_value);
    }

    void unregister(const ice::sonic::String& type, const ice::sonic::String& name) noexcept
    {
        m_ops->unregister(get_handle(), type.get_handle(), name.get_handle());
    }
};

} // namespace ice::sonic
