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
    virtual ~Serde() = default;

    virtual std::expected<ice::String, ice::Status>
    encode(const ice::String& value_json) = 0;

    virtual std::expected<ice::String, ice::Status>
    decode(const ice::String& data) = 0;

    virtual ice::String get_content_type() const = 0;
    virtual ice::String get_format_name() const = 0;

    TF_Serde* get_generic_vtable() {
        static TF_Serde vtable = {
            .struct_size = sizeof(TF_Serde),
            .destroy = [](void* ctx) {
                delete ctx_as<Serde>(ctx);
            },
            .get_content_type = [](void* ctx, TF_String* out) {
                ctx_as<Serde>(ctx)->get_content_type().to_c(out);
            },
            .get_format_name = [](void* ctx, TF_String* out) {
                ctx_as<Serde>(ctx)->get_format_name().to_c(out);
            },
            .encode = [](void* ctx, const TF_TString* value_json, TF_TString* out_encoded, TF_Status* status) {
                auto res = ctx_as<Serde>(ctx)->encode(ice::String::create(value_json));
                if (!res) { res.error().to_c(status); return; }
                res->to_c(out_encoded);
            },
            .decode = [](void* ctx, const TF_TString* data, TF_TString* out_json, TF_Status* status) {
                auto res = ctx_as<Serde>(ctx)->decode(ice::String::create(data));
                if (!res) { res.error().to_c(status); return; }
                res->to_c(out_json);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
