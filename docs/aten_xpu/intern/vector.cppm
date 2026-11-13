module;

#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

export module aten_xpu_intern:vector;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// A growable vector of byte erased elements; set_element_size must come first.
class Vector : public ice::builder::Vector
{
public:
    Vector(const ::TF_StatusOps* status_ops, const ::TF_StringOps* string_ops) noexcept :
        ice::builder::Vector{status_ops},
        m_failure{string_ops}
    {
    }

    ~Vector() override = default;
    Vector(const Vector&) = delete;
    Vector& operator=(const Vector&) = delete;
    Vector(Vector&&) = delete;
    Vector& operator=(Vector&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void set_element_size(std::size_t element_size) noexcept override
    {
        m_element_size = element_size;
        m_bytes.clear();
    }

    void push_back(const void* value) noexcept override
    {
        if (m_element_size == 0) {
            return;
        }

        const auto* first = static_cast<const std::byte*>(value);
        m_bytes.insert(m_bytes.end(), first, first + m_element_size);
    }

    void get(
        std::size_t index,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (!in_range(index)) {
            m_failure.fail(out_status, ice::sonic::Code::k_out_of_range, "index out of range");
            return;
        }

        *out_value = m_bytes.data() + index * m_element_size;
    }

    void set(
        std::size_t index,
        const void* value,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (!in_range(index)) {
            m_failure.fail(out_status, ice::sonic::Code::k_out_of_range, "index out of range");
            return;
        }

        std::memcpy(m_bytes.data() + index * m_element_size, value, m_element_size);
    }

    void size(std::size_t& out_size) noexcept override
    {
        out_size = m_element_size == 0 ? 0 : m_bytes.size() / m_element_size;
    }

    void capacity(std::size_t& out_capacity) noexcept override
    {
        out_capacity = m_element_size == 0 ? 0 : m_bytes.capacity() / m_element_size;
    }

    void reserve(std::size_t new_capacity, const ice::sonic::Status& out_status) noexcept override
    {
        try {
            m_bytes.reserve(new_capacity * m_element_size);
        } catch (const std::exception&) {
            m_failure.fail(out_status, ice::sonic::Code::k_resource_exhausted, "reserve failed");
        }
    }

    void data(void** out_data) noexcept override
    {
        *out_data = m_bytes.data();
    }

private:
    bool in_range(std::size_t index) const noexcept
    {
        return m_element_size != 0 && (index + 1) * m_element_size <= m_bytes.size();
    }

    Failure m_failure;
    std::size_t m_element_size = 0;
    std::vector<std::byte> m_bytes;
};

} // namespace aten_xpu
