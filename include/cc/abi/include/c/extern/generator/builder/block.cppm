// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/block.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/block.h"

export module cc_abi_builder_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFGeneratorBlockOps
{
public:
    static TFGeneratorBlockOps* create(void* ctx) noexcept
    {
        return static_cast<TFGeneratorBlockOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorBlockOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGeneratorBlockOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorBlockOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> add_node(
        const ice::sonic::TFGeneratorDefinitionOps& definition,
        const ice::sonic::TFGeneratorNodeOps& out_node
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_node(int index, const ice::sonic::TFGeneratorNodeOps& out_node) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> list_nodes(TF_Tensor** out_nodes) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_name(const ice::sonic::TF_StringOps& name) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TFGeneratorBlockOps* get_generic_vtable()
    {
        static TFGeneratorBlockOps vtable = {
            .struct_size = TF_ENERATORBLOCK_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFGeneratorBlockOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFGeneratorBlockOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .add_node =
                [](TFGeneratorBlock* block,
                   TFGeneratorDefinition* definition,
                   TFGeneratorNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorBlockOps::create(block);
                auto res = self->add_node(
                    ice::sonic::TFGeneratorDefinitionOps::wrap(definition),
                    ice::sonic::TFGeneratorNodeOps::wrap(out_node)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_node =
                [](TFGeneratorBlock* block,
                   int index,
                   TFGeneratorNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorBlockOps::create(block);
                auto res = self->get_node(index, ice::sonic::TFGeneratorNodeOps::wrap(out_node));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_nodes =
                [](TFGeneratorBlock* block, TF_Tensor** out_nodes, TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorBlockOps::create(block);
                auto res = self->list_nodes(out_nodes);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_name =
                [](TFGeneratorBlock* block, const TF_String* name) noexcept
            {
                auto* self = TFGeneratorBlockOps::create(block);
                auto res = self->set_name(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
