// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/span/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/span/span.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_span;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Span
{
public:
    static Span* create(void* ctx) noexcept
    {
        return static_cast<Span*>(ctx);
    }

    template<typename HandleT>
    static Span* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Span*>(handle);
    }

    virtual ~Span() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    new_span(void* data, size_t count, size_t element_size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get(size_t index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> data() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    subspan(size_t offset, size_t count) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Span* get_generic_vtable()
    {
        static TF_Span vtable = {
            .struct_size = TF_SPAN_STRUCT_SIZE,
            .new_span =
                [](void* plugin_context, void* data, size_t count, size_t element_size) noexcept
            {
                auto* self = Span::create(plugin_context);
                auto res = self->new_span(data, count, element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get =
                [](const TF_Span_Handle* span, size_t index) noexcept
            {
                auto* self = Span::create(span);
                auto res = self->get(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_Span_Handle* span) noexcept
            {
                auto* self = Span::create(span);
                auto res = self->size();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .data =
                [](const TF_Span_Handle* span) noexcept
            {
                auto* self = Span::create(span);
                auto res = self->data();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .subspan =
                [](const TF_Span_Handle* span, size_t offset, size_t count) noexcept
            {
                auto* self = Span::create(span);
                auto res = self->subspan(offset, count);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Span::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
