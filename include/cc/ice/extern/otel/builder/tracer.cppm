// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/tracer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/span.h"
#include "include/c/extern/otel/tracer.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_otel_builder:tracer;

import std;
import cc_ice_extern_otel_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFOtelTracerOps
{
public:
    explicit TFOtelTracerOps(
        const ::TFOtelSpanOps* TFOtelSpanOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFOtelSpanOps_ops = TFOtelSpanOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void start_span(
        const ice::sonic::String& name,
        int kind,
        const ice::sonic::TFOtelSpanOps& out_span,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFOtelTracer*)) noexcept
    {
        m_vtable = ::TFOtelTracerOps{
            .struct_size = TF_OFFSET_OF_END(::TFOtelTracerOps, start_span),

            .create = create,
            .destroy =
                [](TFOtelTracer* handle) noexcept
            {
                auto& self = TFOtelTracerOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFOtelTracer* tracer, TF_String* out_name) noexcept
            {
                auto& self = TFOtelTracerOps::from_handle(tracer);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .start_span =
                [](TFOtelTracer* tracer,
                   const TF_String* name,
                   int kind,
                   TFOtelSpan* out_span,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFOtelTracerOps::from_handle(tracer);
                self.start_span(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    kind,
                    self.wrap(std::type_identity<ice::sonic::TFOtelSpanOps>{}, out_span),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TFOtelSpanOps
    wrap(std::type_identity<ice::sonic::TFOtelSpanOps>, const ::TFOtelSpan* handle) const noexcept
    {
        return ice::sonic::TFOtelSpanOps{m_TFOtelSpanOps_ops, const_cast<::TFOtelSpan*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TFOtelTracerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFOtelTracer& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFOtelTracerOps*>(&m_vtable));
    }

private:
    ::TFOtelTracerOps m_vtable;
    ::TFOtelTracer m_handle;

    const ::TFOtelSpanOps* m_TFOtelSpanOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
