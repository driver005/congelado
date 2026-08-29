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
    // Recover the Events instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Events* create(void* ctx) noexcept
    {
        return static_cast<Events*>(ctx);
    }

    virtual ~Events() = default;

    virtual [[nodiscard]] std::expected<void, ice::Status>
    publish(const ice::String& event_name, const ice::String& payload_json) = 0;

    virtual ice::String get_name() const = 0;

    static TF_Events* get_generic_vtable()
    {
        static TF_Events vtable = {
            .struct_size = sizeof(TF_Events),
            .destroy =
                [](void* plugin_context)
            {
                delete Events::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out)
            {
                auto* self = Events::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            },
            .publish =
                [](void* plugin_context,
                   const TF_TString* event_name,
                   const TF_TString* payload_json,
                   TF_Status* status)
            {
                auto* self = Events::create(plugin_context);
                auto res = self->publish(
                    ice::String::create(event_name),
                    ice::String::create(payload_json)
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
