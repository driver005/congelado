// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/forward_list/forward_list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/forward_list/forward_list.h"

export module cc_ice_intern_forward_list_sonic:forward_list;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ForwardListOps : public ice::sonic::Runtime<TF_ForwardListOps, TF_ForwardListOps>
{
public:
    explicit TF_ForwardListOps(TF_ForwardListOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "forward_list";

    void set_element_size(size_t element_size) noexcept
    {
        m_ops->set_element_size(get_handle(), element_size);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    push_front(const void* value, TFForwardListNode* out_node) noexcept
    {
        ice::sonic::Status status;
        m_ops->push_front(get_handle(), value, out_node, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void erase_after(TFForwardListNode* node) noexcept
    {
        m_ops->erase_after(get_handle(), node);
    }

    void for_each(TF_ForwardListVisitor visitor, void* capture) noexcept
    {
        m_ops->for_each(get_handle(), visitor, capture);
    }

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }
};

} // namespace ice::sonic
