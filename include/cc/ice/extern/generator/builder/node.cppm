// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/node.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/node.h"

export module cc_ice_builder_generator:node;

import std;

export namespace ice::builder {

class TFGeneratorNodeOps
{
public:
    static TFGeneratorNodeOps* create(void* ctx) noexcept
    {
        return static_cast<TFGeneratorNodeOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorNodeOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGeneratorNodeOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorNodeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_operand(int index, const ice::sonic::TF_StringOps& var_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_output_name(int index, const ice::sonic::TF_StringOps& var_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_attr(
        const ice::sonic::TF_StringOps& name,
        const void* value,
        size_t value_size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_operand(int index, const ice::sonic::TF_StringOps& out_operand) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_output_name(int index, const ice::sonic::TF_StringOps& out_output_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_definition(const ice::sonic::TFGeneratorDefinitionOps& out_definition) noexcept = 0;

    static TFGeneratorNodeOps* get_generic_vtable()
    {
        static TFGeneratorNodeOps vtable = {
            .struct_size = TF_ENERATORNODE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFGeneratorNodeOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFGeneratorNodeOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .set_operand =
                [](TFGeneratorNode* node_context,
                   int index,
                   const TF_String* var_name,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorNodeOps::create(node_context);
                auto res = self->set_operand(index, ice::sonic::TF_StringOps::wrap(var_name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_output_name =
                [](TFGeneratorNode* node_context,
                   int index,
                   const TF_String* var_name,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorNodeOps::create(node_context);
                auto res = self->set_output_name(index, ice::sonic::TF_StringOps::wrap(var_name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_attr =
                [](TFGeneratorNode* node_context,
                   const TF_String* name,
                   const void* value,
                   size_t value_size,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorNodeOps::create(node_context);
                auto res = self->set_attr(ice::sonic::TF_StringOps::wrap(name), value, value_size);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_operand =
                [](TFGeneratorNode* node_context, int index, TF_String* out_operand) noexcept
            {
                auto* self = TFGeneratorNodeOps::create(node_context);
                auto res = self->get_operand(index, ice::sonic::TF_StringOps::wrap(out_operand));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_output_name =
                [](TFGeneratorNode* node_context, int index, TF_String* out_output_name) noexcept
            {
                auto* self = TFGeneratorNodeOps::create(node_context);
                auto res =
                    self->get_output_name(index, ice::sonic::TF_StringOps::wrap(out_output_name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_definition =
                [](TFGeneratorNode* node_context,
                   TFGeneratorDefinition* out_definition,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorNodeOps::create(node_context);
                auto res = self->get_definition(
                    ice::sonic::TFGeneratorDefinitionOps::wrap(out_definition)
                );
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
