// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/tracer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/tracer.h"

export module cc_ice_extern_otel_builder:tracer;

import std;

export namespace ice::builder {

class TFOtelTracerOps
{
public:
    TFOtelTracerOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFOtelTracerOps(const TFOtelTracerOps&) = delete;
    TFOtelTracerOps& operator=(const TFOtelTracerOps&) = delete;

    static TFOtelTracerOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFOtelTracerOps*>(ctx);
    }

    template<typename HandleT>
    static TFOtelTracerOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFOtelTracerOps*>(handle->plugin_data);
    }

    virtual ~TFOtelTracerOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> start_span(
        const ice::sonic::String& name,
        int kind,
        const ice::sonic::TFOtelSpanOps& out_span
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFOtelTracerOps{
            .struct_size = TF_TELTRACER_STRUCT_SIZE,
            .destroy =
                [](TFOtelTracer* tracer) noexcept
            {
                TFOtelTracerOps::from_handle(tracer).destroy();
            },
            .get_name =
                [](TFOtelTracer* tracer, TF_String* out_name) noexcept
            {
                TFOtelTracerOps::from_handle(tracer).get_name(ice::sonic::String::wrap(out_name));
            },
            .start_span =
                [](TFOtelTracer* tracer,
                   const TF_String* name,
                   int kind,
                   TFOtelSpan* out_span,
                   TF_Status* out_status) noexcept
            {
                auto res = TFOtelTracerOps::from_handle(tracer).start_span(
                    ice::sonic::String::wrap(name),
                    kind,
                    ice::sonic::TFOtelSpanOps::wrap(out_span)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFOtelTracerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFOtelTracer& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFOtelTracerOps m_vtable;
    TFOtelTracer m_handle;
};

} // namespace ice::builder
