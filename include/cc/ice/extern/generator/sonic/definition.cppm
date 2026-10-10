// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/definition.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/definition.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_generator_sonic:definition;

import std;
import :attribute;
import :parameter;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGeneratorDefinitionOps :
    public ice::sonic::Runtime<::TFGeneratorDefinitionOps, ::TFGeneratorDefinition>
{
public:
    TFGeneratorDefinitionOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFGeneratorDefinitionOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFGeneratorDefinition* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFGeneratorDefinitionOps(const ::TFGeneratorDefinitionOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGeneratorDefinitionOps(
        const ::TFGeneratorDefinitionOps* ops,
        ::TFGeneratorDefinition* handle
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

    void set_name(const ice::sonic::String& name) const noexcept
    {
        m_ops->set_name(get_handle(), name.get_handle());
    }

    void set_summary(const ice::sonic::String& summary) const noexcept
    {
        m_ops->set_summary(get_handle(), summary.get_handle());
    }

    void set_description(const ice::sonic::String& description) const noexcept
    {
        m_ops->set_description(get_handle(), description.get_handle());
    }

    void add_input(
        const ice::sonic::TFGeneratorParameterOps& input,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_input(get_handle(), input.get_handle(), out_status.get_handle());
    }

    void add_output(
        const ice::sonic::TFGeneratorParameterOps& output,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_output(get_handle(), output.get_handle(), out_status.get_handle());
    }

    void add_attr(
        const ice::sonic::TFGeneratorAttributeOps& attr,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_attr(get_handle(), attr.get_handle(), out_status.get_handle());
    }

    void get_summary(const ice::sonic::String& out_summary) const noexcept
    {
        m_ops->get_summary(get_handle(), out_summary.get_handle());
    }

    void get_description(const ice::sonic::String& out_description) const noexcept
    {
        m_ops->get_description(get_handle(), out_description.get_handle());
    }

    void list_inputs(TF_Tensor** out_inputs, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->list_inputs(get_handle(), out_inputs, out_status.get_handle());
    }

    void list_outputs(TF_Tensor** out_outputs, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->list_outputs(get_handle(), out_outputs, out_status.get_handle());
    }

    void list_attrs(TF_Tensor** out_attrs, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->list_attrs(get_handle(), out_attrs, out_status.get_handle());
    }
};

} // namespace ice::sonic
