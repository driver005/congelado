// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/block.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/block.h"
#include "include/c/extern/parser/node.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_parser_builder:block;

import std;
import cc_ice_extern_parser_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class ParserBlock
{
public:
    explicit ParserBlock(
        const ::TFParserNodeOps* ParserNode_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_ParserNode_ops = ParserNode_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    ParserBlock(const ParserBlock&) = delete;
    ParserBlock& operator=(const ParserBlock&) = delete;

    static ParserBlock& from_handle(void* ctx) noexcept
    {
        return *static_cast<ParserBlock*>(ctx);
    }

    template<typename HandleT>
    static ParserBlock& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<ParserBlock*>(handle->plugin_data);
    }

    virtual ~ParserBlock() = default;
    virtual void destroy() noexcept = 0;
    virtual void
    get_name(const ice::sonic::String& out_name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_node_count(int& out_count, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_node(
        int index,
        const ice::sonic::ParserNode& out_node,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserBlock*)) noexcept
    {
        m_vtable = ::TFParserBlockOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserBlockOps, get_node),

            .create = create,
            .destroy =
                [](::TFParserBlock* handle) noexcept
            {
                auto& self = ParserBlock::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TFParserBlock* block, ::TF_String* out_name, ::TF_Status* out_status) noexcept
            {
                auto& self = ParserBlock::from_handle(block);
                self.get_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_node_count =
                [](::TFParserBlock* block, int* out_count, ::TF_Status* out_status) noexcept
            {
                auto& self = ParserBlock::from_handle(block);
                self.get_node_count(
                    *out_count,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_node =
                [](::TFParserBlock* block,
                   int index,
                   ::TFParserNode* out_node,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ParserBlock::from_handle(block);
                self.get_node(
                    index,
                    self.wrap(std::type_identity<ice::sonic::ParserNode>{}, out_node),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::ParserNode
    wrap(std::type_identity<ice::sonic::ParserNode>, const ::TFParserNode* handle) const noexcept
    {
        return ice::sonic::ParserNode{m_ParserNode_ops, const_cast<::TFParserNode*>(handle)};
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

    const ::TFParserBlockOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFParserBlock& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFParserBlockOps*>(&m_vtable));
    }

private:
    ::TFParserBlockOps m_vtable;
    ::TFParserBlock m_handle;

    const ::TFParserNodeOps* m_ParserNode_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
