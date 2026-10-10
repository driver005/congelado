// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/forward_list/forward_list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/forward_list/forward_list.h"

export module cc_ice_intern_forward_list_sonic:forward_list;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_ForwardListOps : public ice::sonic::Runtime<::TF_ForwardListOps, ::TF_ForwardList>
{
public:
    TF_ForwardListOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_ForwardListOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_ForwardList* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_ForwardListOps(const ::TF_ForwardListOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ForwardListOps(const ::TF_ForwardListOps* ops, ::TF_ForwardList* handle) noexcept :
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

    void push_front(
        const void* value,
        TFForwardListNode* out_node,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->push_front(get_handle(), value, out_node, out_status.get_handle());
    }

    void erase_after(TFForwardListNode* node) const noexcept
    {
        m_ops->erase_after(get_handle(), node);
    }

    void for_each(TF_ForwardListVisitor visitor, void* capture) const noexcept
    {
        m_ops->for_each(get_handle(), visitor, capture);
    }
};

} // namespace ice::sonic
