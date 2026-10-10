// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/node.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/node.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_parser_sonic:node;

import std;
import :attribute;
import :definition;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFParserNodeOps : public ice::sonic::Runtime<::TFParserNodeOps, ::TFParserNode>
{
public:
    TFParserNodeOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFParserNodeOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFParserNode* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFParserNodeOps(const ::TFParserNodeOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFParserNodeOps(const ::TFParserNodeOps* ops, ::TFParserNode* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(
        const ice::sonic::String& out_name,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle(), out_status.get_handle());
    }

    void get_op_type(
        const ice::sonic::String& out_op_type,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_op_type(get_handle(), out_op_type.get_handle(), out_status.get_handle());
    }

    void get_attribute_count(int* out_count, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->get_attribute_count(get_handle(), out_count, out_status.get_handle());
    }

    void get_attribute(
        int index,
        const ice::sonic::TFParserAttributeOps& out_attribute,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attribute(
            get_handle(),
            index,
            out_attribute.get_handle(),
            out_status.get_handle()
        );
    }

    void get_definition(
        const ice::sonic::TFParserDefinitionOps& out_definition,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_definition(get_handle(), out_definition.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
