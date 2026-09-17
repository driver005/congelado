// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/otel/meter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/otel/meter.h"

export module cc_abi_builder_otel;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFOtelMeterOps
{
public:
    static TFOtelMeterOps* create(void* ctx) noexcept
    {
        return static_cast<TFOtelMeterOps*>(ctx);
    }

    template<typename HandleT>
    static TFOtelMeterOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFOtelMeterOps*>(handle->plugin_data);
    }

    virtual ~TFOtelMeterOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> create_counter(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_StringOps& description,
        const ice::sonic::TF_StringOps& unit,
        const ice::sonic::TFOtelCounterOps& out_counter
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> create_histogram(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_StringOps& description,
        const ice::sonic::TF_StringOps& unit,
        const ice::sonic::TFOtelHistogramOps& out_histogram
    ) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TFOtelMeterOps* get_generic_vtable()
    {
        static TFOtelMeterOps vtable = {
            .struct_size = TF_TELMETER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFOtelMeterOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFOtelMeterOps::create(plugin_context);
                auto result = self->get_name();
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
                auto* self = TFOtelMeterOps::create(meter);
                auto res = self->create_counter(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_StringOps::wrap(description),
                    ice::sonic::TF_StringOps::wrap(unit),
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
                auto* self = TFOtelMeterOps::create(meter);
                auto res = self->create_histogram(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_StringOps::wrap(description),
                    ice::sonic::TF_StringOps::wrap(unit),
                    ice::sonic::TFOtelHistogramOps::wrap(out_histogram)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
