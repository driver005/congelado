module;

#include "c/extern/payload/payload.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_payload;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for a payload backend — pure interface, zero C-ABI/TF_* knowledge, mirrors
// ice::builder::Builder's role. A backend implements this directly and registers a
// factory function pointer into ice::sonic::RegistrationRuntime under type="payload".
class Payload
{
public:
    // Recover the Payload instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Payload* create(void* ctx) noexcept
    {
        return static_cast<Payload*>(ctx);
    }

    virtual ~Payload() = default;

    virtual [[nodiscard]] std::expected<void, ice::Status> write(
        ice::PayloadType type,
        const ice::String& data,
        TF_Payload_CompletionFn completion,
        void* cb_user_data
    ) = 0;

    virtual [[nodiscard]] std::expected<void, ice::Status>
    read(const ice::String& reference, TF_Payload_CompletionFn completion, void* cb_user_data) = 0;

    virtual ice::String get_name() const = 0;

    static TF_Payload* get_generic_vtable()
    {
        static TF_Payload vtable = {
            .struct_size = sizeof(TF_Payload),
            .destroy =
                [](void* plugin_context)
            {
                delete Payload::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out)
            {
                auto* self = Payload::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            },
            .write =
                [](void* plugin_context,
                   TF_Payload_Type type,
                   const TF_TString* data,
                   TF_Payload_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status)
            {
                auto* self = Payload::create(plugin_context);
                ice::PayloadType cpp_type = ice::payload_type_from_c(type);
                auto res =
                    self->write(cpp_type, ice::String::create(data), completion, cb_user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .read =
                [](void* plugin_context,
                   const TF_TString* reference,
                   TF_Payload_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status)
            {
                auto* self = Payload::create(plugin_context);
                auto res = self->read(ice::String::create(reference), completion, cb_user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
