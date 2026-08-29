module;

#include "c/extern/serde/serde.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_serde;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for a serde (encode/decode) backend — pure interface, zero C-ABI/TF_*
// knowledge, mirrors ice::builder::Builder's role.
class Serde
{
public:
    // Recover the Serde instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Serde* create(void* ctx) noexcept
    {
        return static_cast<Serde*>(ctx);
    }

    virtual ~Serde() = default;

    virtual [[nodiscard]] std::expected<ice::String, ice::Status>
    encode(const ice::String& value_json) = 0;

    virtual [[nodiscard]] std::expected<ice::String, ice::Status>
    decode(const ice::String& data) = 0;

    virtual ice::String get_content_type() const = 0;
    virtual ice::String get_format_name() const = 0;

    static TF_Serde* get_generic_vtable()
    {
        static TF_Serde vtable = {
            .struct_size = sizeof(TF_Serde),
            .destroy =
                [](void* plugin_context)
            {
                delete Serde::create(plugin_context);
            },
            .get_content_type =
                [](void* plugin_context, TF_String* out)
            {
                Serde::create(plugin_context)->get_content_type().to_c(out);
            },
            .get_format_name =
                [](void* plugin_context, TF_String* out)
            {
                Serde::create(plugin_context)->get_format_name().to_c(out);
            },
            .encode =
                [](void* plugin_context,
                   const TF_TString* value_json,
                   TF_TString* out_encoded,
                   TF_Status* status)
            {
                auto res = Serde::create(plugin_context)->encode(ice::String::create(value_json));
                if (!res) {
                    res.error().to_c(status);
                    return;
                }
                res->to_c(out_encoded);
            },
            .decode =
                [](void* plugin_context,
                   const TF_TString* data,
                   TF_TString* out_json,
                   TF_Status* status)
            {
                auto res = Serde::create(plugin_context)->decode(ice::String::create(data));
                if (!res) {
                    res.error().to_c(status);
                    return;
                }
                res->to_c(out_json);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
