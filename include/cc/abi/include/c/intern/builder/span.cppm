// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/span.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_SpanOps
{
public:
    static TF_SpanOps* create(void* ctx) noexcept
    {
        return static_cast<TF_SpanOps*>(ctx);
    }

    template<typename HandleT>
    static TF_SpanOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_SpanOps*>(handle->plugin_data);
    }

    virtual ~TF_SpanOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    get(size_t index, const void** out_value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> data(void** out_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    subspan(size_t offset, size_t count, const ice::sonic::TF_SpanOps& out_span) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_SpanOps* get_generic_vtable()
    {
        static TF_SpanOps vtable = {
            .struct_size = TF_SPAN_STRUCT_SIZE,
            .get =
                [](const TF_Span* span,
                   size_t index,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_SpanOps::create(span);
                auto res = self->get(index, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .size =
                [](const TF_Span* span, size_t* out_size) noexcept
            {
                auto* self = TF_SpanOps::create(span);
                auto res = self->size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .data =
                [](const TF_Span* span, void** out_data) noexcept
            {
                auto* self = TF_SpanOps::create(span);
                auto res = self->data(out_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .subspan =
                [](const TF_Span* span,
                   size_t offset,
                   size_t count,
                   TF_Span* out_span,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_SpanOps::create(span);
                auto res = self->subspan(offset, count, ice::sonic::TF_SpanOps::wrap(out_span));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_SpanOps::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
