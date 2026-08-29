module;

#include "c/extern/otel/otel.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_otel;

export import :leaves;
export import :enums;
import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import :leaves;


export namespace ice::builder {

class Tracer
{
public:
    virtual ~Tracer() = default;

    virtual std::expected<std::unique_ptr<Span>, ice::Status>
    start_span(const ice::String& name, int kind) = 0;
};

class Meter
{
public:
    virtual ~Meter() = default;

    virtual std::expected<std::unique_ptr<Counter>, ice::Status> create_counter(
        const ice::String& name,
        const ice::String& description,
        const ice::String& unit
    ) = 0;

    virtual std::expected<std::unique_ptr<Histogram>, ice::Status> create_histogram(
        const ice::String& name,
        const ice::String& description,
        const ice::String& unit
    ) = 0;
};

// Abstract base class for an OpenTelemetry backend — pure interface, zero C-ABI/TF_* knowledge,
// mirrors ice::builder::Builder's role. A backend implements this directly and
// registers a factory function pointer into ice::sonic::RegistrationRuntime under type="otel".
class Otel
{
public:
    virtual ~Otel() = default;

    virtual std::expected<std::unique_ptr<Tracer>, ice::Status> get_tracer() = 0;
    virtual std::expected<std::unique_ptr<Meter>, ice::Status> get_meter() = 0;

    TF_Otel* get_generic_vtable() {
        static TF_Otel vtable = {
            .struct_size = sizeof(TF_Otel),
            .destroy = [](void* ctx) {
                delete ctx_as<Otel>(ctx);
            },
            .get_tracer = [](void* ctx, TF_Status* status) -> void* {
                auto* self = ctx_as<Otel>(ctx);
                auto res = self->get_tracer();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->release();
            },
            .tracer__destroy = [](void* ctx) {
                delete ctx_as<Tracer>(ctx);
            },
            .get_meter = [](void* ctx, TF_Status* status) -> void* {
                auto* self = ctx_as<Otel>(ctx);
                auto res = self->get_meter();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->release();
            },
            .meter__destroy = [](void* ctx) {
                delete ctx_as<Meter>(ctx);
            },
            .tracer__start_span = [](void* ctx, const TF_TString* name, int kind, TF_Status* status) -> void* {
                auto* self = ctx_as<Tracer>(ctx);
                auto res = self->start_span(
                    ice::String::create(name),
                    kind
                );
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->release();
            },
            .span__destroy = [](void* ctx) {
                delete ctx_as<Span>(ctx);
            },
            .span__set_attribute = [](void* ctx, const TF_TString* key, const TF_TString* value, TF_Status* status) {
                auto* self = ctx_as<Span>(ctx);
                auto res = self->set_attribute(
                    ice::String::create(key),
                    ice::String::create(value)
                );
                if (!res) res.error().to_c(status);
            },
            .span__set_status = [](void* ctx, int status_code, const TF_TString* description, TF_Status* status) {
                auto* self = ctx_as<Span>(ctx);
                auto res = self->set_status(
                    status_code,
                    ice::String::create(description)
                );
                if (!res) res.error().to_c(status);
            },
            .span__end = [](void* ctx, TF_Status* status) {
                auto* self = ctx_as<Span>(ctx);
                auto res = self->end();
                if (!res) res.error().to_c(status);
            },
            .meter__create_counter = [](void* ctx, const TF_TString* name, const TF_TString* description, const TF_TString* unit, TF_Status* status) -> void* {
                auto* self = ctx_as<Meter>(ctx);
                auto res = self->create_counter(
                    ice::String::create(name),
                    ice::String::create(description),
                    ice::String::create(unit)
                );
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->release();
            },
            .counter__destroy = [](void* ctx) {
                delete ctx_as<Counter>(ctx);
            },
            .counter__add = [](void* ctx, double value, TF_Status* status) {
                auto* self = ctx_as<Counter>(ctx);
                auto res = self->add(value);
                if (!res) res.error().to_c(status);
            },
            .meter__create_histogram = [](void* ctx, const TF_TString* name, const TF_TString* description, const TF_TString* unit, TF_Status* status) -> void* {
                auto* self = ctx_as<Meter>(ctx);
                auto res = self->create_histogram(
                    ice::String::create(name),
                    ice::String::create(description),
                    ice::String::create(unit)
                );
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->release();
            },
            .histogram__destroy = [](void* ctx) {
                delete ctx_as<Histogram>(ctx);
            },
            .histogram__record = [](void* ctx, double value, TF_Status* status) {
                auto* self = ctx_as<Histogram>(ctx);
                auto res = self->record(value);
                if (!res) res.error().to_c(status);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
