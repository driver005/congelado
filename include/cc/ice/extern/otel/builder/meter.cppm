// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/meter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/counter.h"
#include "include/c/extern/otel/histogram.h"
#include "include/c/extern/otel/meter.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_otel_builder:meter;

import std;
import cc_ice_extern_otel_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class OtelMeter
{
public:
    explicit OtelMeter(
        const ::TFOtelCounterOps* OtelCounter_ops,
        const ::TFOtelHistogramOps* OtelHistogram_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_OtelCounter_ops = OtelCounter_ops;
        m_OtelHistogram_ops = OtelHistogram_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    OtelMeter(const OtelMeter&) = delete;
    OtelMeter& operator=(const OtelMeter&) = delete;

    static OtelMeter& from_handle(void* ctx) noexcept
    {
        return *static_cast<OtelMeter*>(ctx);
    }

    template<typename HandleT>
    static OtelMeter& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<OtelMeter*>(handle->plugin_data);
    }

    virtual ~OtelMeter() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void create_counter(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit,
        const ice::sonic::OtelCounter& out_counter,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void create_histogram(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit,
        const ice::sonic::OtelHistogram& out_histogram,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFOtelMeter*)) noexcept
    {
        m_vtable = ::TFOtelMeterOps{
            .struct_size = TF_OFFSET_OF_END(::TFOtelMeterOps, create_histogram),

            .create = create,
            .destroy =
                [](::TFOtelMeter* handle) noexcept
            {
                auto& self = OtelMeter::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TFOtelMeter* meter, ::TF_String* out_name) noexcept
            {
                auto& self = OtelMeter::from_handle(meter);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .create_counter =
                [](::TFOtelMeter* meter,
                   const ::TF_String* name,
                   const ::TF_String* description,
                   const ::TF_String* unit,
                   ::TFOtelCounter* out_counter,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = OtelMeter::from_handle(meter);
                self.create_counter(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::String>{}, description),
                    self.wrap(std::type_identity<ice::sonic::String>{}, unit),
                    self.wrap(std::type_identity<ice::sonic::OtelCounter>{}, out_counter),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .create_histogram =
                [](::TFOtelMeter* meter,
                   const ::TF_String* name,
                   const ::TF_String* description,
                   const ::TF_String* unit,
                   ::TFOtelHistogram* out_histogram,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = OtelMeter::from_handle(meter);
                self.create_histogram(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::String>{}, description),
                    self.wrap(std::type_identity<ice::sonic::String>{}, unit),
                    self.wrap(std::type_identity<ice::sonic::OtelHistogram>{}, out_histogram),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::OtelCounter
    wrap(std::type_identity<ice::sonic::OtelCounter>, const ::TFOtelCounter* handle) const noexcept
    {
        return ice::sonic::OtelCounter{m_OtelCounter_ops, handle};
    }

    ice::sonic::OtelHistogram wrap(
        std::type_identity<ice::sonic::OtelHistogram>,
        const ::TFOtelHistogram* handle
    ) const noexcept
    {
        return ice::sonic::OtelHistogram{m_OtelHistogram_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TFOtelMeterOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFOtelMeter& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFOtelMeterOps*>(&m_vtable));
    }

private:
    ::TFOtelMeterOps m_vtable;
    ::TFOtelMeter m_handle;

    const ::TFOtelCounterOps* m_OtelCounter_ops{nullptr};

    const ::TFOtelHistogramOps* m_OtelHistogram_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
