module;

#include "c/extern/otel/otel.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_otel;

export import :leaves;
import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Tracer
{
public:
    // Recover the Tracer instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Tracer* create(void* ctx) noexcept
    {
        return static_cast<Tracer*>(ctx);
    }

    virtual ~Tracer() = default;

    [[nodiscard]] virtual std::expected<std::unique_ptr<Span>, ice::Status>
    start_span(const ice::String& name, ice::SpanKind kind) noexcept = 0;
};

class Meter
{
public:
    // Recover the Meter instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Meter* create(void* ctx) noexcept
    {
        return static_cast<Meter*>(ctx);
    }

    virtual ~Meter() = default;

    [[nodiscard]] virtual std::expected<std::unique_ptr<Counter>, ice::Status> create_counter(
        const ice::String& name,
        const ice::String& description,
        const ice::String& unit
    ) noexcept = 0;

    [[nodiscard]] virtual std::expected<std::unique_ptr<Histogram>, ice::Status> create_histogram(
        const ice::String& name,
        const ice::String& description,
        const ice::String& unit
    ) noexcept = 0;
};

// Abstract base class for an OpenTelemetry backend — pure interface, zero C-ABI/TF_* knowledge,
// mirrors ice::builder::Generator's role. A backend implements this directly and
// registers a factory function pointer into ice::sonic::Registration under type="otel".
class Otel
{
public:
    // Recover the Otel instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Otel* create(void* ctx) noexcept
    {
        return static_cast<Otel*>(ctx);
    }

    virtual ~Otel() = default;

    virtual ice::String get_name() const noexcept = 0;

    [[nodiscard]] virtual std::expected<std::unique_ptr<Tracer>, ice::Status> create_tracer() noexcept = 0;
    [[nodiscard]] virtual std::expected<std::unique_ptr<Meter>, ice::Status> create_meter() noexcept = 0;

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
                Otel::create(plugin_context)->get_name().to_c(out);
            },
            .create_tracer = [](void* plugin_context, TF_Status* status) noexcept -> TF_Otel_Tracer*
            {
                auto* self = Otel::create(plugin_context);
                auto res = self->create_tracer();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_Otel_Tracer*>(static_cast<void*>(res->release()));
            },
            .tracer__destroy =
                [](TF_Otel_Tracer* tracer_context) noexcept
            {
                delete Tracer::create(tracer_context);
            },
            .create_meter = [](void* plugin_context, TF_Status* status) noexcept -> TF_Otel_Meter*
            {
                auto* self = Otel::create(plugin_context);
                auto res = self->create_meter();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_Otel_Meter*>(static_cast<void*>(res->release()));
            },
            .meter__destroy =
                [](TF_Otel_Meter* meter_context) noexcept
            {
                delete Meter::create(meter_context);
            },
            .tracer__start_span = [](TF_Otel_Tracer* tracer_context,
                                     const TF_TString* name,
                                     int kind,
                                     TF_Status* status) noexcept -> TF_Otel_Span*
            {
                auto* self = Tracer::create(static_cast<void*>(tracer_context));
                auto res = self->start_span(
                    ice::String::create(name),
                    ice::span_kind_from_c(static_cast<TF_Otel_SpanKind>(kind))
                );
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_Otel_Span*>(static_cast<void*>(res->release()));
            },
            .span__destroy =
                [](TF_Otel_Span* span_context) noexcept
            {
                delete Span::create(span_context);
            },
            .span__set_attribute =
                [](TF_Otel_Span* span_context,
                   const TF_TString* key,
                   const TF_TString* value,
                   TF_Status* status) noexcept
            {
                auto* self = Span::create(span_context);
                auto res =
                    self->set_attribute(ice::String::create(key), ice::String::create(value));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .span__set_status =
                [](TF_Otel_Span* span_context,
                   int status_code,
                   const TF_TString* description,
                   TF_Status* status) noexcept
            {
                auto* self = Span::create(span_context);
                auto res = self->set_status(
                    ice::span_status_from_c(static_cast<TF_Otel_SpanStatus>(status_code)),
                    ice::String::create(description)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .span__end =
                [](TF_Otel_Span* span_context, TF_Status* status) noexcept
            {
                auto* self = Span::create(span_context);
                auto res = self->end();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .meter__create_counter = [](TF_Otel_Meter* meter_context,
                                        const TF_TString* name,
                                        const TF_TString* description,
                                        const TF_TString* unit,
                                        TF_Status* status) noexcept -> TF_Otel_Counter*
            {
                auto* self = Meter::create(meter_context);
                auto res = self->create_counter(
                    ice::String::create(name),
                    ice::String::create(description),
                    ice::String::create(unit)
                );
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_Otel_Counter*>(static_cast<void*>(res->release()));
            },
            .counter__destroy =
                [](TF_Otel_Counter* counter_context) noexcept
            {
                delete Counter::create(counter_context);
            },
            .counter__add =
                [](TF_Otel_Counter* counter_context, double value, TF_Status* status) noexcept
            {
                auto* self = Counter::create(counter_context);
                auto res = self->add(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .meter__create_histogram = [](TF_Otel_Meter* meter_context,
                                          const TF_TString* name,
                                          const TF_TString* description,
                                          const TF_TString* unit,
                                          TF_Status* status) noexcept -> TF_Otel_Histogram*
            {
                auto* self = Meter::create(meter_context);
                auto res = self->create_histogram(
                    ice::String::create(name),
                    ice::String::create(description),
                    ice::String::create(unit)
                );
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_Otel_Histogram*>(static_cast<void*>(res->release()));
            },
            .histogram__destroy =
                [](TF_Otel_Histogram* histogram_context) noexcept
            {
                delete Histogram::create(histogram_context);
            },
            .histogram__record =
                [](TF_Otel_Histogram* histogram_context, double value, TF_Status* status) noexcept
            {
                auto* self = Histogram::create(histogram_context);
                auto res = self->record(value);
                if (!res) {
                    res.error().to_c(status);
                }
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
