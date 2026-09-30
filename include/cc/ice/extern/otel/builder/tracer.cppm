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
    [[nodiscard]] virtual std::expected<void, ice::Status> start_span(
        const ice::sonic::String& name,
        int kind,
        const ice::sonic::TFOtelSpanOps& out_span
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFOtelTracerOps{
            .struct_size = TF_TELTRACER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TFOtelTracerOps>{&TFOtelTracerOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TFOtelTracerOps::from_handle(plugin_context).get_name();
                result.to_c(out);
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

    virtual builder::String get_name() const noexcept = 0;

    const TFOtelTracer& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFOtelTracerOps m_vtable;
    TFOtelTracer m_handle;
};

} // namespace ice::builder
