// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/block.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/block.h"
#include "include/c/extern/generator/definition.h"
#include "include/c/extern/generator/node.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_generator_builder:block;

import std;
import cc_ice_extern_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class GeneratorBlock
{
public:
    explicit GeneratorBlock(
        const ::TFGeneratorDefinitionOps* GeneratorDefinition_ops,
        const ::TFGeneratorNodeOps* GeneratorNode_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_GeneratorDefinition_ops = GeneratorDefinition_ops;
        m_GeneratorNode_ops = GeneratorNode_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    GeneratorBlock(const GeneratorBlock&) = delete;
    GeneratorBlock& operator=(const GeneratorBlock&) = delete;

    static GeneratorBlock& from_handle(void* ctx) noexcept
    {
        return *static_cast<GeneratorBlock*>(ctx);
    }

    template<typename HandleT>
    static GeneratorBlock& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<GeneratorBlock*>(handle->plugin_data);
    }

    virtual ~GeneratorBlock() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void add_node(
        const ice::sonic::GeneratorDefinition& definition,
        const ice::sonic::GeneratorNode& out_node,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_node(
        int index,
        const ice::sonic::GeneratorNode& out_node,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    list_nodes(::TF_Tensor** out_nodes, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void set_name(const ice::sonic::String& name) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGeneratorBlock*)) noexcept
    {
        m_vtable = ::TFGeneratorBlockOps{
            .struct_size = TF_OFFSET_OF_END(::TFGeneratorBlockOps, set_name),

            .create = create,
            .destroy =
                [](::TFGeneratorBlock* handle) noexcept
            {
                auto& self = GeneratorBlock::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TFGeneratorBlock* block, ::TF_String* out_name) noexcept
            {
                auto& self = GeneratorBlock::from_handle(block);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .add_node =
                [](::TFGeneratorBlock* block,
                   ::TFGeneratorDefinition* definition,
                   ::TFGeneratorNode* out_node,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorBlock::from_handle(block);
                self.add_node(
                    self.wrap(std::type_identity<ice::sonic::GeneratorDefinition>{}, definition),
                    self.wrap(std::type_identity<ice::sonic::GeneratorNode>{}, out_node),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_node =
                [](::TFGeneratorBlock* block,
                   int index,
                   ::TFGeneratorNode* out_node,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorBlock::from_handle(block);
                self.get_node(
                    index,
                    self.wrap(std::type_identity<ice::sonic::GeneratorNode>{}, out_node),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_nodes =
                [](::TFGeneratorBlock* block,
                   ::TF_Tensor** out_nodes,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorBlock::from_handle(block);
                self.list_nodes(
                    out_nodes,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_name =
                [](::TFGeneratorBlock* block, const ::TF_String* name) noexcept
            {
                auto& self = GeneratorBlock::from_handle(block);
                self.set_name(self.wrap(std::type_identity<ice::sonic::String>{}, name));
            },

        };
    }

    ice::sonic::GeneratorDefinition wrap(
        std::type_identity<ice::sonic::GeneratorDefinition>,
        const ::TFGeneratorDefinition* handle
    ) const noexcept
    {
        return ice::sonic::GeneratorDefinition{
            m_GeneratorDefinition_ops,
            const_cast<::TFGeneratorDefinition*>(handle)
        };
    }

    ice::sonic::GeneratorNode wrap(
        std::type_identity<ice::sonic::GeneratorNode>,
        const ::TFGeneratorNode* handle
    ) const noexcept
    {
        return ice::sonic::GeneratorNode{
            m_GeneratorNode_ops,
            const_cast<::TFGeneratorNode*>(handle)
        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TFGeneratorBlockOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGeneratorBlock& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFGeneratorBlockOps*>(&m_vtable));
    }

private:
    ::TFGeneratorBlockOps m_vtable;
    ::TFGeneratorBlock m_handle;

    const ::TFGeneratorDefinitionOps* m_GeneratorDefinition_ops{nullptr};

    const ::TFGeneratorNodeOps* m_GeneratorNode_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
