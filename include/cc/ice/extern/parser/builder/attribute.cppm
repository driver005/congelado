// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/attribute.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/attribute.h"

export module cc_ice_extern_parser_builder:attribute;

import std;

export namespace ice::builder {

class TFParserAttributeOps
{
public:
    static TFParserAttributeOps* create(void* ctx) noexcept
    {
        return static_cast<TFParserAttributeOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserAttributeOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFParserAttributeOps*>(handle->plugin_data);
    }

    virtual ~TFParserAttributeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_value(TF_Tensor** out_value) noexcept = 0;

    static TFParserAttributeOps* get_generic_vtable()
    {
        static TFParserAttributeOps vtable = {
            .struct_size = TF_ARSERATTRIBUTE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFParserAttributeOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .get_value =
                [](TFParserAttribute* attribute,
                   TF_Tensor** out_value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFParserAttributeOps::create(attribute);
                auto res = self->get_value(out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
