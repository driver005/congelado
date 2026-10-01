// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/list/list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/list/list.h"

export module cc_ice_intern_list_sonic:list;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_ListOps : public ice::sonic::Runtime<::TF_ListOps, ::TF_List>
{
public:
    template<typename Registry>
    TF_ListOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_ListOps(
        Registry& registry,
        ::TF_List* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_ListOps(const ::TF_ListOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ListOps(const ::TF_ListOps* ops, ::TF_List* handle) noexcept :
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
        TFListNode* out_node,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->push_front(get_handle(), value, out_node, out_status.get_handle());
    }

    void push_back(
        const void* value,
        TFListNode* out_node,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->push_back(get_handle(), value, out_node, out_status.get_handle());
    }

    void erase(TFListNode* node) const noexcept
    {
        m_ops->erase(get_handle(), node);
    }

    void for_each(TF_ListVisitor visitor, void* capture) const noexcept
    {
        m_ops->for_each(get_handle(), visitor, capture);
    }

    void size(size_t* out_size) const noexcept
    {
        m_ops->size(get_handle(), out_size);
    }
};

} // namespace ice::sonic
