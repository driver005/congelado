// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/node.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/node.h"

export module cc_ice_builder_parser:node;

import std;

export namespace ice::builder {

class TFParserNodeOps
{
public:
    static TFParserNodeOps* create(void* ctx) noexcept
    {
        return static_cast<TFParserNodeOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserNodeOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFParserNodeOps*>(handle->plugin_data);
    }

    virtual ~TFParserNodeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_op_type(const ice::sonic::TF_StringOps& out_op_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attribute_count(int* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attribute(int index, const ice::sonic::TFParserAttributeOps& out_attribute) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_definition(const ice::sonic::TFParserDefinitionOps& out_definition) noexcept = 0;

    static TFParserNodeOps* get_generic_vtable()
    {
        static TFParserNodeOps vtable = {
            .struct_size = TF_ARSERNODE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFParserNodeOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .get_op_type =
                [](TFParserNode* node, TF_String* out_op_type, TF_Status* out_status) noexcept
            {
                auto* self = TFParserNodeOps::create(node);
                auto res = self->get_op_type(ice::sonic::TF_StringOps::wrap(out_op_type));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attribute_count =
                [](TFParserNode* node, int* out_count, TF_Status* out_status) noexcept
            {
                auto* self = TFParserNodeOps::create(node);
                auto res = self->get_attribute_count(out_count);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attribute =
                [](TFParserNode* node,
                   int index,
                   TFParserAttribute* out_attribute,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFParserNodeOps::create(node);
                auto res = self->get_attribute(
                    index,
                    ice::sonic::TFParserAttributeOps::wrap(out_attribute)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_definition =
                [](TFParserNode* node,
                   TFParserDefinition* out_definition,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFParserNodeOps::create(node);
                auto res =
                    self->get_definition(ice::sonic::TFParserDefinitionOps::wrap(out_definition));
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
