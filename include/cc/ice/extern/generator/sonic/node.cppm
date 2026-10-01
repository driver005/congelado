// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/node.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/node.h"

export module cc_ice_extern_generator_sonic:node;

import std;
import :definition;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGeneratorNodeOps : public ice::sonic::Runtime<::TFGeneratorNodeOps, ::TFGeneratorNode>
{
public:
    template<typename Registry>
    TFGeneratorNodeOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFGeneratorNodeOps(
        Registry& registry,
        ::TFGeneratorNode* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFGeneratorNodeOps(const ::TFGeneratorNodeOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGeneratorNodeOps(const ::TFGeneratorNodeOps* ops, ::TFGeneratorNode* handle) noexcept :
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

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void set_operand(
        int index,
        const ice::sonic::String& var_name,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_operand(get_handle(), index, var_name.get_handle(), out_status.get_handle());
    }

    void set_output_name(
        int index,
        const ice::sonic::String& var_name,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_output_name(get_handle(), index, var_name.get_handle(), out_status.get_handle());
    }

    void set_attr(
        const ice::sonic::String& name,
        const void* value,
        size_t value_size,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops
            ->set_attr(get_handle(), name.get_handle(), value, value_size, out_status.get_handle());
    }

    void get_operand(int index, const ice::sonic::String& out_operand) const noexcept
    {
        m_ops->get_operand(get_handle(), index, out_operand.get_handle());
    }

    void get_output_name(int index, const ice::sonic::String& out_output_name) const noexcept
    {
        m_ops->get_output_name(get_handle(), index, out_output_name.get_handle());
    }

    void get_definition(
        const ice::sonic::TFGeneratorDefinitionOps& out_definition,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_definition(get_handle(), out_definition.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
