// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/otel/otel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/otel/otel.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_otel;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Otel
{
public:
    static Otel* create(void* ctx) noexcept
    {
        return static_cast<Otel*>(ctx);
    }

    template<typename HandleT>
    static Otel* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Otel*>(handle);
    }

    virtual ~Otel() = default;
    [[nodiscard]] std::expected<void, ice::Status> create_tracer() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> tracer_destroy() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> create_meter() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> meter_destroy() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    tracer_start_span(const ice::sonic::String& name, int kind) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> span_destroy() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    span_set_attribute(const ice::sonic::String& key, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    span_set_status(int status_code, const ice::sonic::String& description) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> span_end() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> meter_create_counter(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> counter_destroy() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> counter_add(double value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> meter_create_histogram(
        const ice::sonic::String& name,
        const ice::sonic::String& description,
        const ice::sonic::String& unit
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> histogram_destroy() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> histogram_record(double value) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Otel* get_generic_vtable()
    {
        static TF_Otel vtable = {
            .struct_size = TF_OTEL_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Otel::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Otel::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .create_tracer =
                [](void* plugin_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Otel::create(plugin_context);
                auto res = self->create_tracer();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tracer_destroy =
                [](TF_Otel_Tracer* tracer_context) noexcept
            {
                auto* self = Otel::create(tracer_context);
                auto res = self->tracer_destroy();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_meter =
                [](void* plugin_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Otel::create(plugin_context);
                auto res = self->create_meter();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .meter_destroy =
                [](TF_Otel_Meter* meter_context) noexcept
            {
                auto* self = Otel::create(meter_context);
                auto res = self->meter_destroy();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tracer_start_span =
                [](TF_Otel_Tracer* tracer_context,
                   const TF_String_Handle* name,
                   int kind,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Otel::create(tracer_context);
                auto res = self->tracer_start_span(ice::sonic::String::wrap(name), kind);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .span_destroy =
                [](TF_Otel_Span* span_context) noexcept
            {
                auto* self = Otel::create(span_context);
                auto res = self->span_destroy();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .span_set_attribute =
                [](TF_Otel_Span* span_context,
                   const TF_String_Handle* key,
                   const TF_String_Handle* value,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Otel::create(span_context);
                auto res = self->span_set_attribute(
                    ice::sonic::String::wrap(key),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .span_set_status =
                [](TF_Otel_Span* span_context,
                   int status_code,
                   const TF_String_Handle* description,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Otel::create(span_context);
                auto res =
                    self->span_set_status(status_code, ice::sonic::String::wrap(description));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .span_end =
                [](TF_Otel_Span* span_context, TF_Status_Handle* status) noexcept
            {
                auto* self = Otel::create(span_context);
                auto res = self->span_end();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .meter_create_counter =
                [](TF_Otel_Meter* meter_context,
                   const TF_String_Handle* name,
                   const TF_String_Handle* description,
                   const TF_String_Handle* unit,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Otel::create(meter_context);
                auto res = self->meter_create_counter(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(description),
                    ice::sonic::String::wrap(unit)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .counter_destroy =
                [](TF_Otel_Counter* counter_context) noexcept
            {
                auto* self = Otel::create(counter_context);
                auto res = self->counter_destroy();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .counter_add =
                [](TF_Otel_Counter* counter_context,
                   double value,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Otel::create(counter_context);
                auto res = self->counter_add(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .meter_create_histogram =
                [](TF_Otel_Meter* meter_context,
                   const TF_String_Handle* name,
                   const TF_String_Handle* description,
                   const TF_String_Handle* unit,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Otel::create(meter_context);
                auto res = self->meter_create_histogram(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(description),
                    ice::sonic::String::wrap(unit)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .histogram_destroy =
                [](TF_Otel_Histogram* histogram_context) noexcept
            {
                auto* self = Otel::create(histogram_context);
                auto res = self->histogram_destroy();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .histogram_record =
                [](TF_Otel_Histogram* histogram_context,
                   double value,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Otel::create(histogram_context);
                auto res = self->histogram_record(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
