// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/definition.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/definition.h"

export module cc_ice_extern_parser_builder:definition;

import std;

export namespace ice::builder {

class TFParserDefinitionOps
{
public:
    static TFParserDefinitionOps* create(void* ctx) noexcept
    {
        return static_cast<TFParserDefinitionOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserDefinitionOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFParserDefinitionOps*>(handle->plugin_data);
    }

    virtual ~TFParserDefinitionOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_source_file(const ice::sonic::TF_StringOps& out_source_file) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_line_number(int* out_line_number) noexcept = 0;

    static TFParserDefinitionOps* get_generic_vtable()
    {
        static TFParserDefinitionOps vtable = {
            .struct_size = TF_ARSERDEFINITION_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFParserDefinitionOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .get_source_file =
                [](TFParserDefinition* definition,
                   TF_String* out_source_file,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFParserDefinitionOps::create(definition);
                auto res = self->get_source_file(ice::sonic::TF_StringOps::wrap(out_source_file));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_line_number =
                [](TFParserDefinition* definition,
                   int* out_line_number,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFParserDefinitionOps::create(definition);
                auto res = self->get_line_number(out_line_number);
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
