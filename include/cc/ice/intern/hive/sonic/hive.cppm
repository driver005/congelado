// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/hive/hive.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/hive/hive.h"

export module cc_ice_intern_hive_sonic:hive;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_HiveOps : public ice::sonic::Runtime<::TF_HiveOps, ::TF_Hive>
{
public:
    TF_HiveOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_HiveOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Hive* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_HiveOps(const ::TF_HiveOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_HiveOps(const ::TF_HiveOps* ops, ::TF_Hive* handle) noexcept :
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

    void set_element_size(size_t element_size) const noexcept
    {
        m_ops->set_element_size(get_handle(), element_size);
    }

    void insert(
        const void* value,
        TFHiveSlot* out_slot,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->insert(get_handle(), value, out_slot, out_status.get_handle());
    }

    void erase(TFHiveSlot* slot, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->erase(get_handle(), slot, out_status.get_handle());
    }

    void get(
        const TFHiveSlot* slot,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get(get_handle(), slot, out_value, out_status.get_handle());
    }

    void for_each(TF_HiveVisitor visitor, void* capture) const noexcept
    {
        m_ops->for_each(get_handle(), visitor, capture);
    }

    void size(size_t* out_size) const noexcept
    {
        m_ops->size(get_handle(), out_size);
    }
};

} // namespace ice::sonic
