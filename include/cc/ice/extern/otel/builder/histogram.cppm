// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/histogram.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/histogram.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_otel_builder:histogram;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFOtelHistogramOps
{
public:
    explicit TFOtelHistogramOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFOtelHistogramOps(const TFOtelHistogramOps&) = delete;
    TFOtelHistogramOps& operator=(const TFOtelHistogramOps&) = delete;

    static TFOtelHistogramOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFOtelHistogramOps*>(ctx);
    }

    template<typename HandleT>
    static TFOtelHistogramOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFOtelHistogramOps*>(handle->plugin_data);
    }

    virtual ~TFOtelHistogramOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void record(double value, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFOtelHistogram*)) noexcept
    {
        m_vtable = ::TFOtelHistogramOps{
            .struct_size = TF_OFFSET_OF_END(::TFOtelHistogramOps, record),

            .create = create,
            .destroy =
                [](TFOtelHistogram* handle) noexcept
            {
                auto& self = TFOtelHistogramOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFOtelHistogram* histogram, TF_String* out_name) noexcept
            {
                auto& self = TFOtelHistogramOps::from_handle(histogram);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .record =
                [](TFOtelHistogram* histogram, double value, TF_Status* out_status) noexcept
            {
                auto& self = TFOtelHistogramOps::from_handle(histogram);
                self.record(value, self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
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

    const ::TFOtelHistogramOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFOtelHistogram& get_handle() const noexcept
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
            const_cast<::TFOtelHistogramOps*>(&m_vtable)
        );
    }

private:
    ::TFOtelHistogramOps m_vtable;
    ::TFOtelHistogram m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
