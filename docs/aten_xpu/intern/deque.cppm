module;

#include "include/c/intern/deque.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:deque;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// A double ended queue of byte erased elements; each element keeps its own stable storage.
class Deque : public ice::builder::Deque
{
public:
    Deque(const ::TF_StatusOps* status_ops, const ::TF_StringOps* string_ops) noexcept :
        ice::builder::Deque{status_ops},
        m_failure{string_ops}
    {
    }

    ~Deque() override = default;
    Deque(const Deque&) = delete;
    Deque& operator=(const Deque&) = delete;
    Deque(Deque&&) = delete;
    Deque& operator=(Deque&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void set_element_size(std::size_t element_size) noexcept override
    {
        m_element_size = element_size;
        m_items.clear();
    }

    void push_front(const void* value) noexcept override
    {
        if (m_element_size != 0) {
            m_items.push_front(copy_of(value));
        }
    }

    void push_back(const void* value) noexcept override
    {
        if (m_element_size != 0) {
            m_items.push_back(copy_of(value));
        }
    }

    void pop_front() noexcept override
    {
        if (!m_items.empty()) {
            m_items.pop_front();
        }
    }

    void pop_back() noexcept override
    {
        if (!m_items.empty()) {
            m_items.pop_back();
        }
    }

    void get(
        std::size_t index,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (index >= m_items.size()) {
            m_failure.fail(out_status, ice::sonic::Code::k_out_of_range, "index out of range");
            return;
        }

        *out_value = m_items[index].data();
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
    std::deque<std::vector<std::byte>> m_items;
};

} // namespace aten_xpu
