// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/node.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/attribute.h"
#include "include/c/extern/parser/definition.h"
#include "include/c/extern/parser/node.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_parser_builder:node;

import std;
import cc_ice_extern_parser_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class ParserNode
{
public:
    explicit ParserNode(
        const ::TFParserAttributeOps* ParserAttribute_ops,
        const ::TFParserDefinitionOps* ParserDefinition_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_ParserAttribute_ops = ParserAttribute_ops;
        m_ParserDefinition_ops = ParserDefinition_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    ParserNode(const ParserNode&) = delete;
    ParserNode& operator=(const ParserNode&) = delete;

    static ParserNode& from_handle(void* ctx) noexcept
    {
        return *static_cast<ParserNode*>(ctx);
    }

    template<typename HandleT>
    static ParserNode& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<ParserNode*>(handle->plugin_data);
    }

    virtual ~ParserNode() = default;
    virtual void destroy() noexcept = 0;
    virtual void
    get_name(const ice::sonic::String& out_name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_op_type(
        const ice::sonic::String& out_op_type,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    get_attribute_count(int& out_count, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_attribute(
        int index,
        const ice::sonic::ParserAttribute& out_attribute,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_definition(
        const ice::sonic::ParserDefinition& out_definition,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFParserNode*)) noexcept
    {
        m_vtable = ::TFParserNodeOps{
            .struct_size = TF_OFFSET_OF_END(::TFParserNodeOps, get_definition),

            .create = create,
            .destroy =
                [](::TFParserNode* handle) noexcept
            {
                auto& self = ParserNode::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TFParserNode* node, ::TF_String* out_name, ::TF_Status* out_status) noexcept
            {
                auto& self = ParserNode::from_handle(node);
                self.get_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_op_type =
                [](::TFParserNode* node, ::TF_String* out_op_type, ::TF_Status* out_status) noexcept
            {
                auto& self = ParserNode::from_handle(node);
                self.get_op_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_op_type),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attribute_count =
                [](::TFParserNode* node, int* out_count, ::TF_Status* out_status) noexcept
            {
                auto& self = ParserNode::from_handle(node);
                self.get_attribute_count(
                    *out_count,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attribute =
                [](::TFParserNode* node,
                   int index,
                   ::TFParserAttribute* out_attribute,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ParserNode::from_handle(node);
                self.get_attribute(
                    index,
                    self.wrap(std::type_identity<ice::sonic::ParserAttribute>{}, out_attribute),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_definition =
                [](::TFParserNode* node,
                   ::TFParserDefinition* out_definition,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ParserNode::from_handle(node);
                self.get_definition(
                    self.wrap(std::type_identity<ice::sonic::ParserDefinition>{}, out_definition),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::ParserAttribute wrap(
        std::type_identity<ice::sonic::ParserAttribute>,
        const ::TFParserAttribute* handle
    ) const noexcept
    {
        return ice::sonic::ParserAttribute{
            m_ParserAttribute_ops,
            const_cast<::TFParserAttribute*>(handle)
        };
    }

    ice::sonic::ParserDefinition wrap(
        std::type_identity<ice::sonic::ParserDefinition>,
        const ::TFParserDefinition* handle
    ) const noexcept
    {
        return ice::sonic::ParserDefinition{
            m_ParserDefinition_ops,
            const_cast<::TFParserDefinition*>(handle)
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

    const ::TFParserNodeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFParserNode& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFParserNodeOps*>(&m_vtable));
    }

private:
    ::TFParserNodeOps m_vtable;
    ::TFParserNode m_handle;

    const ::TFParserAttributeOps* m_ParserAttribute_ops{nullptr};

    const ::TFParserDefinitionOps* m_ParserDefinition_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
