// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/parameter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/parameter.h"

export module cc_ice_extern_parser_builder:parameter;

import std;

export namespace ice::builder {

class TFParserParameterOps
{
public:
    static TFParserParameterOps* create(void* ctx) noexcept
    {
        return static_cast<TFParserParameterOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserParameterOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFParserParameterOps*>(handle->plugin_data);
    }

    virtual ~TFParserParameterOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_typeinfo(const ice::sonic::TFParserTypeInfoOps& out_typeinfo) noexcept = 0;

    static TFParserParameterOps* get_generic_vtable()
    {
        static TFParserParameterOps vtable = {
            .struct_size = TF_ARSERPARAMETER_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFParserParameterOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .get_typeinfo =
                [](TFParserParameter* parameter,
                   TFParserTypeInfo* out_typeinfo,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFParserParameterOps::create(parameter);
                auto res = self->get_typeinfo(ice::sonic::TFParserTypeInfoOps::wrap(out_typeinfo));
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
