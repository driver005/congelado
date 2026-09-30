// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/block.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/block.h"

export module cc_ice_extern_generator_builder:block;

import std;

export namespace ice::builder {

class TFGeneratorBlockOps
{
public:
    TFGeneratorBlockOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFGeneratorBlockOps(const TFGeneratorBlockOps&) = delete;
    TFGeneratorBlockOps& operator=(const TFGeneratorBlockOps&) = delete;

    static TFGeneratorBlockOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorBlockOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorBlockOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorBlockOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorBlockOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> add_node(
        const ice::sonic::TFGeneratorDefinitionOps& definition,
        const ice::sonic::TFGeneratorNodeOps& out_node
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_node(int index, const ice::sonic::TFGeneratorNodeOps& out_node) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    list_nodes(TF_Tensor** out_nodes) noexcept = 0;
    virtual void set_name(const ice::sonic::String& name) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGeneratorBlockOps{
            .struct_size = TF_ENERATORBLOCK_STRUCT_SIZE,
            .destroy =
                [](TFGeneratorBlock* block) noexcept
            {
                TFGeneratorBlockOps::from_handle(block).destroy();
            },
            .get_name =
                [](TFGeneratorBlock* block, TF_String* out_name) noexcept
            {
                TFGeneratorBlockOps::from_handle(block).get_name(
                    ice::sonic::String::wrap(out_name)
                );
            },
            .add_node =
                [](TFGeneratorBlock* block,
                   TFGeneratorDefinition* definition,
                   TFGeneratorNode* out_node,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorBlockOps::from_handle(block).add_node(
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
                auto res = TFGeneratorBlockOps::from_handle(block).get_node(
                    index,
                    ice::sonic::TFGeneratorNodeOps::wrap(out_node)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_nodes =
                [](TFGeneratorBlock* block, TF_Tensor** out_nodes, TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorBlockOps::from_handle(block).list_nodes(out_nodes);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_name =
                [](TFGeneratorBlock* block, const TF_String* name) noexcept
            {
                TFGeneratorBlockOps::from_handle(block).set_name(ice::sonic::String::wrap(name));
            },

        };
    }

    const ::TFGeneratorBlockOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFGeneratorBlock& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGeneratorBlockOps m_vtable;
    TFGeneratorBlock m_handle;
};

} // namespace ice::builder
