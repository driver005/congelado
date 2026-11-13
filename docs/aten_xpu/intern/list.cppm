module;

#include "include/c/intern/list/list.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:list;

import std;
import cc_ice_intern_list_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// A doubly linked list of byte erased elements; a node handle points at the element's storage.
class List : public ice::builder::List
{
public:
    List(const ::TF_StatusOps* status_ops, const ::TF_StringOps* string_ops) noexcept :
        ice::builder::List{status_ops},
        m_failure{string_ops}
    {
    }

    ~List() override = default;
    List(const List&) = delete;
    List& operator=(const List&) = delete;
    List(List&&) = delete;
    List& operator=(List&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void set_element_size(std::size_t element_size) noexcept override
    {
        m_element_size = element_size;
        m_items.clear();
    }

    void push_front(
        const void* value,
        ::TFListNode* out_node,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (m_element_size == 0) {
            m_failure.fail(out_status, ice::sonic::Code::k_failed_precondition, "no element size");
            return;
        }

        out_node->plugin_data = &m_items.emplace_front(copy_of(value));
    }

    void push_back(
        const void* value,
        ::TFListNode* out_node,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (m_element_size == 0) {
            m_failure.fail(out_status, ice::sonic::Code::k_failed_precondition, "no element size");
            return;
        }

        out_node->plugin_data = &m_items.emplace_back(copy_of(value));
    }

    void erase(::TFListNode* node) noexcept override
    {
        const auto found = std::ranges::find_if(
            m_items,
            [node](const std::vector<std::byte>& item)
            {
                return &item == node->plugin_data;
            }
        );
        if (found != m_items.end()) {
            m_items.erase(found);
        }
    }

    void for_each(TF_ListVisitor visitor, void* capture) noexcept override
    {
        for (const auto& item: m_items) {
            visitor(capture, item.data());
        }
    }

    void size(std::size_t& out_size) noexcept override
    {
        out_size = m_items.size();
    }

private:
    std::vector<std::byte> copy_of(const void* value) const
    {
        const auto* first = static_cast<const std::byte*>(value);

        return std::vector<std::byte>(first, first + m_element_size);
    }

    Failure m_failure;
    std::size_t m_element_size = 0;
    std::list<std::vector<std::byte>> m_items;
};

} // namespace aten_xpu
