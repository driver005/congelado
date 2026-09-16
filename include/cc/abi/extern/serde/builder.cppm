// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/serde/serde.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/serde/serde.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_serde;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Serde
{
public:
    static Serde* create(void* ctx) noexcept
    {
        return static_cast<Serde*>(ctx);
    }

    template<typename HandleT>
    static Serde* create(HandleT* handle) noexcept
    {
        return static_cast<Serde*>(handle->plugin_data);
    }

    virtual ~Serde() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    get_content_type(const ice::sonic::String& out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_format_name(const ice::sonic::String& out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    encode(const ice::sonic::String& value_json, const ice::sonic::String& out_encoded) noexcept =
        0;
    [[nodiscard]] std::expected<void, ice::Status>
    decode(const ice::sonic::String& data, const ice::sonic::String& out_json) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_SerdeOps* get_generic_vtable()
    {
        static TF_SerdeOps vtable = {
            .struct_size = TF_SERDE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Serde::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Serde::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .get_content_type =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Serde::create(plugin_context);
                auto res = self->get_content_type(ice::sonic::String::wrap(out));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_format_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Serde::create(plugin_context);
                auto res = self->get_format_name(ice::sonic::String::wrap(out));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .encode =
                [](void* plugin_context,
                   const TF_String* value_json,
                   TF_String* out_encoded,
                   TF_Status* status) noexcept
            {
                auto* self = Serde::create(plugin_context);
                auto res = self->encode(
                    ice::sonic::String::wrap(value_json),
                    ice::sonic::String::wrap(out_encoded)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .decode =
                [](void* plugin_context,
                   const TF_String* data,
                   TF_String* out_json,
                   TF_Status* status) noexcept
            {
                auto* self = Serde::create(plugin_context);
                auto res = self->decode(
                    ice::sonic::String::wrap(data),
                    ice::sonic::String::wrap(out_json)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
