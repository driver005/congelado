// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/serde/serde.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/serde/serde.h"

export module cc_abi_builder_serde;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_SerdeOps
{
public:
    static TF_SerdeOps* create(void* ctx) noexcept
    {
        return static_cast<TF_SerdeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_SerdeOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_SerdeOps*>(handle->plugin_data);
    }

    virtual ~TF_SerdeOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    get_content_type(const ice::sonic::TF_StringOps& out_content_type) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_format_name(const ice::sonic::TF_StringOps& out_format_name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> encode(
        const ice::sonic::TF_StringOps& value_json,
        const ice::sonic::TF_StringOps& out_encoded
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> decode(
        const ice::sonic::TF_StringOps& data,
        const ice::sonic::TF_StringOps& out_json
    ) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_SerdeOps* get_generic_vtable()
    {
        static TF_SerdeOps vtable = {
            .struct_size = TF_SERDE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_SerdeOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_SerdeOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .get_content_type =
                [](TF_Serde* serde, TF_String* out_content_type) noexcept
            {
                auto* self = TF_SerdeOps::create(serde);
                auto res = self->get_content_type(ice::sonic::TF_StringOps::wrap(out_content_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_format_name =
                [](TF_Serde* serde, TF_String* out_format_name) noexcept
            {
                auto* self = TF_SerdeOps::create(serde);
                auto res = self->get_format_name(ice::sonic::TF_StringOps::wrap(out_format_name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .encode =
                [](TF_Serde* serde,
                   const TF_String* value_json,
                   TF_String* out_encoded,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_SerdeOps::create(serde);
                auto res = self->encode(
                    ice::sonic::TF_StringOps::wrap(value_json),
                    ice::sonic::TF_StringOps::wrap(out_encoded)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .decode =
                [](TF_Serde* serde,
                   const TF_String* data,
                   TF_String* out_json,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_SerdeOps::create(serde);
                auto res = self->decode(
                    ice::sonic::TF_StringOps::wrap(data),
                    ice::sonic::TF_StringOps::wrap(out_json)
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
