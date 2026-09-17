// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/tracer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/tracer.h"

export module cc_abi_builder_otel;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFOtelTracerOps
{
public:
    static TFOtelTracerOps* create(void* ctx) noexcept
    {
        return static_cast<TFOtelTracerOps*>(ctx);
    }

    template<typename HandleT>
    static TFOtelTracerOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFOtelTracerOps*>(handle->plugin_data);
    }

    virtual ~TFOtelTracerOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> start_span(
        const ice::sonic::TF_StringOps& name,
        int kind,
        const ice::sonic::TFOtelSpanOps& out_span
    ) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TFOtelTracerOps* get_generic_vtable()
    {
        static TFOtelTracerOps vtable = {
            .struct_size = TF_TELTRACER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFOtelTracerOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFOtelTracerOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .start_span =
                [](TFOtelTracer* tracer,
                   const TF_String* name,
                   int kind,
                   TFOtelSpan* out_span,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFOtelTracerOps::create(tracer);
                auto res = self->start_span(
                    ice::sonic::TF_StringOps::wrap(name),
                    kind,
                    ice::sonic::TFOtelSpanOps::wrap(out_span)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
