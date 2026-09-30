// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/span.h"

export module cc_ice_intern_builder:span;

import std;

export namespace ice::builder {

class TF_SpanOps
{
public:
    TF_SpanOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_SpanOps(const TF_SpanOps&) = delete;
    TF_SpanOps& operator=(const TF_SpanOps&) = delete;

    static TF_SpanOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_SpanOps*>(ctx);
    }

    template<typename HandleT>
    static TF_SpanOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_SpanOps*>(handle->plugin_data);
    }

    virtual ~TF_SpanOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get(size_t index, const void** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> data(void** out_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    subspan(size_t offset, size_t count, const ice::sonic::TF_SpanOps& out_span) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_SpanOps{
            .struct_size = TF_SPAN_STRUCT_SIZE,
            .get =
                [](const TF_Span* span,
                   size_t index,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SpanOps::from_handle(span).get(index, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .size =
                [](const TF_Span* span, size_t* out_size) noexcept
            {
                auto res = TF_SpanOps::from_handle(span).size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .data =
                [](const TF_Span* span, void** out_data) noexcept
            {
                auto res = TF_SpanOps::from_handle(span).data(out_data);
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
                auto res = TF_SpanOps::from_handle(span)
                               .subspan(offset, count, ice::sonic::TF_SpanOps::wrap(out_span));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_SpanOps>{&TF_SpanOps::from_handle(plugin_context)};
            },

        };
    }

    const ::TF_SpanOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Span& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_SpanOps m_vtable;
    TF_Span m_handle;
};

} // namespace ice::builder
