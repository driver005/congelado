// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/span.h"
#include "include/c/intern/status.h"

export module cc_ice_intern_builder:span;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_SpanOps
{
public:
    explicit TF_SpanOps(
        const ::TF_SpanOps* TF_SpanOps_ops,
        const ::TF_StatusOps* Status_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_SpanOps_ops = TF_SpanOps_ops;
        m_Status_ops = Status_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void
    get(size_t index, const void** out_value, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;
    virtual void data(void** out_data) noexcept = 0;
    virtual void subspan(
        size_t offset,
        size_t count,
        const ice::sonic::TF_SpanOps& out_span,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Span*)) noexcept
    {
        m_vtable = ::TF_SpanOps{
            .struct_size = TF_OFFSET_OF_END(::TF_SpanOps, subspan),

            .create = create,
            .destroy =
                [](TF_Span* handle) noexcept
            {
                auto& self = TF_SpanOps::from_handle(handle);
                self.destroy();
            },
            .get =
                [](const TF_Span* span,
                   size_t index,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SpanOps::from_handle(span);
                self.get(
                    index,
                    out_value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .size =
                [](const TF_Span* span, size_t* out_size) noexcept
            {
                auto& self = TF_SpanOps::from_handle(span);
                self.size(out_size);
            },
            .data =
                [](const TF_Span* span, void** out_data) noexcept
            {
                auto& self = TF_SpanOps::from_handle(span);
                self.data(out_data);
            },
            .subspan =
                [](const TF_Span* span,
                   size_t offset,
                   size_t count,
                   TF_Span* out_span,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SpanOps::from_handle(span);
                self.subspan(
                    offset,
                    count,
                    self.wrap(std::type_identity<ice::sonic::TF_SpanOps>{}, out_span),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_SpanOps
    wrap(std::type_identity<ice::sonic::TF_SpanOps>, const ::TF_Span* handle) const noexcept
    {
        return ice::sonic::TF_SpanOps{m_TF_SpanOps_ops, const_cast<::TF_Span*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_SpanOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Span& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_SpanOps*>(&m_vtable));
    }

private:
    ::TF_SpanOps m_vtable;
    ::TF_Span m_handle;

    const ::TF_SpanOps* m_TF_SpanOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
