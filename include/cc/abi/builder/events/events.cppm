module;

#include "c/extern/events/events.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_events;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for an events backend — pure interface, zero C-ABI/TF_* knowledge, mirrors
// ice::builder::Builder's role. A backend implements this directly and registers a
// factory function pointer into ice::sonic::RegistrationRuntime under type="events".
class Events
{
public:
    virtual ~Events() = default;

    virtual std::expected<void, ice::Status>
    publish(const ice::String& event_name, const ice::String& payload_json) = 0;

    virtual ice::String get_name() const = 0;

    TF_Events* get_generic_vtable()
    {
        static TF_Events vtable = {
            .struct_size = sizeof(TF_Events),
            .destroy =
                [](void* ctx) {
                    delete ctx_as<Events>(ctx);
                },
            .get_name =
                [](void* ctx, TF_String* out) {
                    auto* self = ctx_as<Events>(ctx);
                    auto name = self->get_name();
                    name.to_c(out);
                },
            .publish =
                [](void* ctx, const TF_TString* event_name, const TF_TString* payload_json,
                   TF_Status* status) {
                    auto* self = ctx_as<Events>(ctx);
                    auto res = self->publish(
                        ice::String::create(event_name), ice::String::create(payload_json)
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                }
        };
        return &vtable;
    }
};

} // namespace ice::builder
