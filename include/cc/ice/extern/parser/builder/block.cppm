// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/block.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/block.h"

export module cc_ice_extern_parser_builder:block;

import std;

export namespace ice::builder {

class TFParserBlockOps
{
public:
    static TFParserBlockOps* create(void* ctx) noexcept
    {
        return static_cast<TFParserBlockOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserBlockOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFParserBlockOps*>(handle->plugin_data);
    }

    virtual ~TFParserBlockOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_node_count(int* out_count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_node(int index, const ice::sonic::TFParserNodeOps& out_node) noexcept = 0;

    static TFParserBlockOps* get_generic_vtable()
    {
        static TFParserBlockOps vtable = {
            .struct_size = TF_ARSERBLOCK_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFParserBlockOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .get_node_count =
                [](TFParserBlock* block, int* out_count, TF_Status* out_status) noexcept
            {
                auto* self = TFParserBlockOps::create(block);
                auto res = self->get_node_count(out_count);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_node =
                [](TFParserBlock* block,
                   int index,
                   TFParserNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFParserBlockOps::create(block);
                auto res = self->get_node(index, ice::sonic::TFParserNodeOps::wrap(out_node));
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
