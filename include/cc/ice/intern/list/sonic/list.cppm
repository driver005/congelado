// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/list/list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/list/list.h"

export module cc_ice_intern_list_sonic:list;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ListOps : public ice::sonic::Runtime<TF_ListOps, TF_ListOps>
{
public:
    explicit TF_ListOps(TF_ListOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "list";

    void set_element_size(size_t element_size) noexcept
    {
        m_ops->set_element_size(get_handle(), element_size);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    push_front(const void* value, TFListNode* out_node) noexcept
    {
        ice::sonic::Status status;
        m_ops->push_front(get_handle(), value, out_node, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    push_back(const void* value, TFListNode* out_node) noexcept
    {
        ice::sonic::Status status;
        m_ops->push_back(get_handle(), value, out_node, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void erase(TFListNode* node) noexcept
    {
        m_ops->erase(get_handle(), node);
    }

    void for_each(TF_ListVisitor visitor, void* capture) noexcept
    {
        m_ops->for_each(get_handle(), visitor, capture);
    }

    void size(size_t* out_size) noexcept
    {
        m_ops->size(get_handle(), out_size);
    }

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }
};

} // namespace ice::sonic
