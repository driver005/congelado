// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/function.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/function.h"

export module cc_ice_extern_generator_sonic:function;

import std;
import :attribute;
import :block;
import :definition;
import :parameter;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGeneratorFunctionOps :
    public ice::sonic::Runtime<::TFGeneratorFunctionOps, ::TFGeneratorFunction>
{
public:
    template<typename Registry>
    TFGeneratorFunctionOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFGeneratorFunctionOps(
        Registry& registry,
        ::TFGeneratorFunction* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFGeneratorFunctionOps(const ::TFGeneratorFunctionOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGeneratorFunctionOps(
        const ::TFGeneratorFunctionOps* ops,
        ::TFGeneratorFunction* handle
    ) noexcept :
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

    void add_parameter(
        const ice::sonic::TFGeneratorParameterOps& parameter,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_parameter(get_handle(), parameter.get_handle(), out_status.get_handle());
    }

    void add_attribute(
        const ice::sonic::TFGeneratorAttributeOps& attribute,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_attribute(get_handle(), attribute.get_handle(), out_status.get_handle());
    }

    void add_definition(
        const ice::sonic::TFGeneratorDefinitionOps& definition,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_definition(get_handle(), definition.get_handle(), out_status.get_handle());
    }

    void add_block(
        const ice::sonic::TFGeneratorBlockOps& block,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_block(get_handle(), block.get_handle(), out_status.get_handle());
    }

    void get_parameter(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorParameterOps& out_parameter,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_parameter(
            get_handle(),
            name.get_handle(),
            out_parameter.get_handle(),
            out_status.get_handle()
        );
    }

    void get_attribute(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorAttributeOps& out_attribute,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_attribute(
            get_handle(),
            name.get_handle(),
            out_attribute.get_handle(),
            out_status.get_handle()
        );
    }

    void get_definition(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorDefinitionOps& out_definition,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_definition(
            get_handle(),
            name.get_handle(),
            out_definition.get_handle(),
            out_status.get_handle()
        );
    }

    void get_block(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorBlockOps& out_block,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_block(
            get_handle(),
            name.get_handle(),
            out_block.get_handle(),
            out_status.get_handle()
        );
    }

    void list_parameters(
        TF_Tensor** out_parameters,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list_parameters(get_handle(), out_parameters, out_status.get_handle());
    }

    void list_attributes(
        TF_Tensor** out_attributes,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list_attributes(get_handle(), out_attributes, out_status.get_handle());
    }

    void list_definitions(
        TF_Tensor** out_definitions,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list_definitions(get_handle(), out_definitions, out_status.get_handle());
    }

    void list_blocks(TF_Tensor** out_blocks, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->list_blocks(get_handle(), out_blocks, out_status.get_handle());
    }

    void finish(
        const ice::sonic::TF_TensorOps& outputs,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->finish(get_handle(), outputs.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
