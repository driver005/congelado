// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/span.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/span.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_otel_builder:span;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFOtelSpanOps
{
public:
    explicit TFOtelSpanOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void set_attribute(
        const ice::sonic::String& key,
        const ice::sonic::String& value,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_status(
        int status_code,
        const ice::sonic::String& description,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void end(const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFOtelSpan*)) noexcept
    {
        m_vtable = ::TFOtelSpanOps{
            .struct_size = TF_OFFSET_OF_END(::TFOtelSpanOps, end),

            .create = create,
            .destroy =
                [](TFOtelSpan* handle) noexcept
            {
                auto& self = TFOtelSpanOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFOtelSpan* span, TF_String* out_name) noexcept
            {
                auto& self = TFOtelSpanOps::from_handle(span);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .set_attribute =
                [](TFOtelSpan* span,
                   const TF_String* key,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFOtelSpanOps::from_handle(span);
                self.set_attribute(
                    self.wrap(std::type_identity<ice::sonic::String>{}, key),
                    self.wrap(std::type_identity<ice::sonic::String>{}, value),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_status =
                [](TFOtelSpan* span,
                   int status_code,
                   const TF_String* description,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFOtelSpanOps::from_handle(span);
                self.set_status(
                    status_code,
                    self.wrap(std::type_identity<ice::sonic::String>{}, description),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .end =
                [](TFOtelSpan* span, TF_Status* out_status) noexcept
            {
                auto& self = TFOtelSpanOps::from_handle(span);
                self.end(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },

        };
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

    const ::TFOtelSpanOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFOtelSpan& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TFOtelSpanOps*>(&m_vtable)
        );
    }

private:
    ::TFOtelSpanOps m_vtable;
    ::TFOtelSpan m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
