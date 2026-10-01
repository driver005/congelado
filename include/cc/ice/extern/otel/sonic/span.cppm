// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/span.h"

export module cc_ice_extern_otel_sonic:span;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFOtelSpanOps : public ice::sonic::Runtime<::TFOtelSpanOps, ::TFOtelSpan>
{
public:
    template<typename Registry>
    TFOtelSpanOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFOtelSpanOps(
        Registry& registry,
        ::TFOtelSpan* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFOtelSpanOps(const ::TFOtelSpanOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFOtelSpanOps(const ::TFOtelSpanOps* ops, ::TFOtelSpan* handle) noexcept :
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

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void set_attribute(
        const ice::sonic::String& key,
        const ice::sonic::String& value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_attribute(
            get_handle(),
            key.get_handle(),
            value.get_handle(),
            out_status.get_handle()
        );
    }

    void set_status(
        int status_code,
        const ice::sonic::String& description,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_status(
            get_handle(),
            status_code,
            description.get_handle(),
            out_status.get_handle()
        );
    }

    void end(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->end(get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
