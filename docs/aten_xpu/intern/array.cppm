module;

#include "include/c/intern/array.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:array;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// A fixed count array of byte erased elements; set_element_size and set_count size it.
class Array : public ice::builder::Array
{
public:
    Array(const ::TF_StatusOps* status_ops, const ::TF_StringOps* string_ops) noexcept :
        ice::builder::Array{status_ops},
        m_failure{string_ops}
    {
    }

    ~Array() override = default;
    Array(const Array&) = delete;
    Array& operator=(const Array&) = delete;
    Array(Array&&) = delete;
    Array& operator=(Array&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void set_element_size(std::size_t element_size) noexcept override
    {
        m_element_size = element_size;
        m_bytes.assign(m_count * m_element_size, std::byte{0});
    }

    void set_count(std::size_t count) noexcept override
    {
        m_count = count;
        m_bytes.assign(m_count * m_element_size, std::byte{0});
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
        out_size = m_count;
    }

    void data(void** out_data) noexcept override
    {
        *out_data = m_bytes.data();
    }

private:
    bool in_range(std::size_t index) const noexcept
    {
        return m_element_size != 0 && index < m_count;
    }

    Failure m_failure;
    std::size_t m_element_size = 0;
    std::size_t m_count = 0;
    std::vector<std::byte> m_bytes;
};

} // namespace aten_xpu
