// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/list/list.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/list/list.h"

export module cc_abi_sonic_list;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class List : public ice::sonic::Runtime<List, TF_List>
{
public:
    explicit List(TF_List* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "list";

    [[nodiscard]] std::expected<void, ice::Status> new_list(size_t element_size) noexcept
    {
        ice::Status status;
        m_ops->new_list(get_handle(), element_size, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> push_front(const void* value) noexcept
    {
        ice::Status status;
        m_ops->push_front(get_handle(), value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> push_back(const void* value) noexcept
    {
        ice::Status status;
        m_ops->push_back(get_handle(), value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> erase(TF_List_Node* node) noexcept
    {
        ice::Status status;
        m_ops->erase(get_handle(), node, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    for_each(TF_ListVisitor visitor, void* capture) noexcept
    {
        ice::Status status;
        m_ops->for_each(get_handle(), visitor, capture, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> size() noexcept
    {
        ice::Status status;
        m_ops->size(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
