// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/span.h"

export module cc_ice_builder_otel:span;

import std;

export namespace ice::builder {

class TFOtelSpanOps
{
public:
    static TFOtelSpanOps* create(void* ctx) noexcept
    {
        return static_cast<TFOtelSpanOps*>(ctx);
    }

    template<typename HandleT>
    static TFOtelSpanOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFOtelSpanOps*>(handle->plugin_data);
    }

    virtual ~TFOtelSpanOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_attribute(
        const ice::sonic::TF_StringOps& key,
        const ice::sonic::TF_StringOps& value
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_status(int status_code, const ice::sonic::TF_StringOps& description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> end() noexcept = 0;

    static TFOtelSpanOps* get_generic_vtable()
    {
        static TFOtelSpanOps vtable = {
            .struct_size = TF_TELSPAN_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFOtelSpanOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFOtelSpanOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .set_attribute =
                [](TFOtelSpan* span,
                   const TF_String* key,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFOtelSpanOps::create(span);
                auto res = self->set_attribute(
                    ice::sonic::TF_StringOps::wrap(key),
                    ice::sonic::TF_StringOps::wrap(value)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_status =
                [](TFOtelSpan* span,
                   int status_code,
                   const TF_String* description,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFOtelSpanOps::create(span);
                auto res =
                    self->set_status(status_code, ice::sonic::TF_StringOps::wrap(description));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .end =
                [](TFOtelSpan* span, TF_Status* out_status) noexcept
            {
                auto* self = TFOtelSpanOps::create(span);
                auto res = self->end();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
