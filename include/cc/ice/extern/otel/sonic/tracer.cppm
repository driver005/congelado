// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/tracer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/tracer.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_otel_sonic:tracer;

import std;
import :span;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class OtelTracer : public ice::sonic::Runtime<::TFOtelTracerOps, ::TFOtelTracer>
{
public:
    OtelTracer(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    OtelTracer(
        const ice::sonic::Registration& registry,
        const ::TFOtelTracer& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit OtelTracer(const ::TFOtelTracerOps* ops) noexcept :
        Runtime(ops)
    {
    }

    OtelTracer(const ::TFOtelTracerOps* ops, const ::TFOtelTracer* handle) noexcept :
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

    void start_span(
        const ice::sonic::String& name,
        int kind,
        const ice::sonic::OtelSpan& out_span,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->start_span(
            get_handle(),
            name.get_handle(),
            kind,
            out_span.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
