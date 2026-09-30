// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/meter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/meter.h"

export module cc_ice_extern_otel_builder:meter;

import std;

export namespace ice::builder {

class TFOtelMeterOps
{
public:
    TFOtelMeterOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFOtelMeterOps(const TFOtelMeterOps&) = delete;
    TFOtelMeterOps& operator=(const TFOtelMeterOps&) = delete;

    static TFOtelMeterOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFOtelMeterOps*>(ctx);
    }

    template<typename HandleT>
    static TFOtelMeterOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFOtelMeterOps*>(handle->plugin_data);
    }

    virtual ~TFOtelMeterOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_counter(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit,
        const ice::sonic::TFOtelCounterOps& out_counter
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_histogram(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit,
        const ice::sonic::TFOtelHistogramOps& out_histogram
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFOtelMeterOps{
            .struct_size = TF_TELMETER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TFOtelMeterOps>{&TFOtelMeterOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TFOtelMeterOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .create_counter =
                [](TFOtelMeter* meter,
                   const TF_String* name,
                   const TF_String* description,
                   const TF_String* unit,
                   TFOtelCounter* out_counter,
                   TF_Status* out_status) noexcept
            {
                auto res = TFOtelMeterOps::from_handle(meter).create_counter(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(description),
                    ice::sonic::String::wrap(unit),
                    ice::sonic::TFOtelCounterOps::wrap(out_counter)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .create_histogram =
                [](TFOtelMeter* meter,
                   const TF_String* name,
                   const TF_String* description,
                   const TF_String* unit,
                   TFOtelHistogram* out_histogram,
                   TF_Status* out_status) noexcept
            {
                auto res = TFOtelMeterOps::from_handle(meter).create_histogram(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(description),
                    ice::sonic::String::wrap(unit),
                    ice::sonic::TFOtelHistogramOps::wrap(out_histogram)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFOtelMeterOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TFOtelMeter& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFOtelMeterOps m_vtable;
    TFOtelMeter m_handle;
};

} // namespace ice::builder
