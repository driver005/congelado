module;

#include "include/c/intern/span.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:span;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// A non-owning view of element_size byte elements; bind() points it at memory the caller keeps
// alive.
class Span : public ice::builder::Span
{
public:
    Span(
        const ::TF_SpanOps* span_ops,
        const ::TF_StatusOps* status_ops,
        const ::TF_StringOps* string_ops
    ) noexcept :
        ice::builder::Span{span_ops, status_ops},
        m_failure{string_ops}
    {
    }

    ~Span() override = default;
    Span(const Span&) = delete;
    Span& operator=(const Span&) = delete;
    Span(Span&&) = delete;
    Span& operator=(Span&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void bind(const void* data, std::size_t count, std::size_t element_size) noexcept
    {
        m_data = static_cast<const std::byte*>(data);
        m_count = count;
        m_element_size = element_size;
    }

    void get(
        std::size_t index,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (index >= m_count) {
            m_failure.fail(out_status, ice::sonic::Code::k_out_of_range, "index out of range");
            return;
        }

        *out_value = m_data + index * m_element_size;
    }

    void size(std::size_t& out_size) noexcept override
    {
        out_size = m_count;
    }

    void data(void** out_data) noexcept override
    {
        *out_data = const_cast<std::byte*>(m_data);
    }

    // Binds `out_span` only when it is a Span; any other span gets an error.
    void subspan(
        std::size_t offset,
        std::size_t count,
        const ice::sonic::Span& out_span,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (offset > m_count || count > m_count - offset) {
            m_failure.fail(out_status, ice::sonic::Code::k_out_of_range, "subspan out of range");
            return;
        }

        auto* target =
            out_span.get_handle()->plugin_data == nullptr
                ? nullptr
                : dynamic_cast<Span*>(&ice::builder::Span::from_handle(out_span.get_handle()));
        if (target == nullptr) {
            m_failure.fail(
                out_status,
                ice::sonic::Code::k_invalid_argument,
                "subspan target is not a host span"
            );
            return;
        }

        target->bind(m_data + offset * m_element_size, count, m_element_size);
    }

private:
    Failure m_failure;
    const std::byte* m_data = nullptr;
    std::size_t m_count = 0;
    std::size_t m_element_size = 0;
};

} // namespace aten_xpu
