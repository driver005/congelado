// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/attrtype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/attrtype.h"

export module cc_ice_intern_sonic:attrtype;

import std;
import :runtime;
import :tstring;

export namespace ice::sonic {

class TF_AttrTypeOps : public ice::sonic::Runtime<::TF_AttrTypeOps, ::TF_AttrType>
{
public:
    TF_AttrTypeOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_AttrTypeOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_AttrType* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_AttrTypeOps(const ::TF_AttrTypeOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_AttrTypeOps(const ::TF_AttrTypeOps* ops, ::TF_AttrType* handle) noexcept :
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

    void attrtype_name(TFAttrTypeEnum type, const ice::sonic::String& out_type_name) const noexcept
    {
        m_ops->attrtype_name(get_handle(), type, out_type_name.get_handle());
    }
};

} // namespace ice::sonic
