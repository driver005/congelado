module;

#include "include/c/intern/forward_list/forward_list.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:forward_list;

import std;
import cc_ice_intern_forward_list_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// A singly linked list of byte erased elements; a null node means "before the first element".
class ForwardList : public ice::builder::ForwardList
{
public:
    ForwardList(const ::TF_StatusOps* status_ops, const ::TF_StringOps* string_ops) noexcept :
        ice::builder::ForwardList{status_ops},
        m_failure{string_ops}
    {
    }

    ~ForwardList() override = default;
    ForwardList(const ForwardList&) = delete;
    ForwardList& operator=(const ForwardList&) = delete;
    ForwardList(ForwardList&&) = delete;
    ForwardList& operator=(ForwardList&&) = delete;

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
        ::TFForwardListNode* out_node,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (m_element_size == 0) {
            m_failure.fail(out_status, ice::sonic::Code::k_failed_precondition, "no element size");
            return;
        }

        const auto* first = static_cast<const std::byte*>(value);
        out_node->plugin_data =
            &m_items.emplace_front(std::vector<std::byte>(first, first + m_element_size));
    }

    void erase_after(::TFForwardListNode* node) noexcept override
    {
        auto before = m_items.before_begin();
        if (node != nullptr && node->plugin_data != nullptr) {
            before = find_before(node->plugin_data);
            if (before == m_items.end()) {
                return;
            }
            ++before;
        }

        if (std::next(before) != m_items.end()) {
            m_items.erase_after(before);
        }
    }

    void for_each(TF_ForwardListVisitor visitor, void* capture) noexcept override
    {
        for (const auto& item: m_items) {
            visitor(capture, item.data());
        }
    }

private:
    // The iterator whose successor is `storage`, or end() when it is not in the list.
    std::forward_list<std::vector<std::byte>>::iterator find_before(void* storage)
    {
        auto previous = m_items.before_begin();
        for (auto current = m_items.begin(); current != m_items.end(); ++current, ++previous) {
            if (&*current == storage) {
                return previous;
            }
        }

        return m_items.end();
    }

    Failure m_failure;
    std::size_t m_element_size = 0;
    std::forward_list<std::vector<std::byte>> m_items;
};

} // namespace aten_xpu
