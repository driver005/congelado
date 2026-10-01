// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/span.h"

export module cc_ice_intern_sonic:span;

import std;
import :runtime;
import :status;
import :tstring;

export namespace ice::sonic {

class TF_SpanOps : public ice::sonic::Runtime<::TF_SpanOps, ::TF_Span>
{
public:
    template<typename Registry>
    TF_SpanOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_SpanOps(
        Registry& registry,
        ::TF_Span* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_SpanOps(const ::TF_SpanOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_SpanOps(const ::TF_SpanOps* ops, ::TF_Span* handle) noexcept :
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

    void get(
        size_t index,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get(get_handle(), index, out_value, out_status.get_handle());
    }

    void size(size_t* out_size) const noexcept
    {
        m_ops->size(get_handle(), out_size);
    }

    void data(void** out_data) const noexcept
    {
        m_ops->data(get_handle(), out_data);
    }

    void subspan(
        size_t offset,
        size_t count,
        const ice::sonic::TF_SpanOps& out_span,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->subspan(get_handle(), offset, count, out_span.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
