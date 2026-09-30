// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/span.h"

export module cc_ice_extern_otel_builder:span;

import std;

export namespace ice::builder {

class TFOtelSpanOps
{
public:
    TFOtelSpanOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFOtelSpanOps(const TFOtelSpanOps&) = delete;
    TFOtelSpanOps& operator=(const TFOtelSpanOps&) = delete;

    static TFOtelSpanOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFOtelSpanOps*>(ctx);
    }

    template<typename HandleT>
    static TFOtelSpanOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFOtelSpanOps*>(handle->plugin_data);
    }

    virtual ~TFOtelSpanOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_attribute(const ice::sonic::String& key, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_status(int status_code, const ice::sonic::String& description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> end() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFOtelSpanOps{
            .struct_size = TF_TELSPAN_STRUCT_SIZE,
            .destroy =
                [](TFOtelSpan* span) noexcept
            {
                TFOtelSpanOps::from_handle(span).destroy();
            },
            .get_name =
                [](TFOtelSpan* span, TF_String* out_name) noexcept
            {
                TFOtelSpanOps::from_handle(span).get_name(ice::sonic::String::wrap(out_name));
            },
            .set_attribute =
                [](TFOtelSpan* span,
                   const TF_String* key,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto res = TFOtelSpanOps::from_handle(span).set_attribute(
                    ice::sonic::String::wrap(key),
                    ice::sonic::String::wrap(value)
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
                auto res = TFOtelSpanOps::from_handle(span).set_status(
                    status_code,
                    ice::sonic::String::wrap(description)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .end =
                [](TFOtelSpan* span, TF_Status* out_status) noexcept
            {
                auto res = TFOtelSpanOps::from_handle(span).end();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFOtelSpanOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFOtelSpan& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFOtelSpanOps m_vtable;
    TFOtelSpan m_handle;
};

} // namespace ice::builder
