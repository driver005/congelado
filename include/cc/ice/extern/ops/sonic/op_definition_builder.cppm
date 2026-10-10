// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/op_definition_builder.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/op_definition_builder.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_ops_sonic:op_definition_builder;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_OpDefinitionBuilderOps :
    public ice::sonic::Runtime<::TF_OpDefinitionBuilderOps, ::TF_OpDefinitionBuilder>
{
public:
    TF_OpDefinitionBuilderOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_OpDefinitionBuilderOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_OpDefinitionBuilder* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_OpDefinitionBuilderOps(const ::TF_OpDefinitionBuilderOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_OpDefinitionBuilderOps(
        const ::TF_OpDefinitionBuilderOps* ops,
        ::TF_OpDefinitionBuilder* handle
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

    void add_attr(const ice::sonic::String& attr_spec) const noexcept
    {
        m_ops->add_attr(get_handle(), attr_spec.get_handle());
    }

    void add_input(const ice::sonic::String& input_spec) const noexcept
    {
        m_ops->add_input(get_handle(), input_spec.get_handle());
    }

    void add_output(const ice::sonic::String& output_spec) const noexcept
    {
        m_ops->add_output(get_handle(), output_spec.get_handle());
    }

    void set_is_commutative(_Bool is_commutative) const noexcept
    {
        m_ops->set_is_commutative(get_handle(), is_commutative);
    }

    void set_is_aggregate(_Bool is_aggregate) const noexcept
    {
        m_ops->set_is_aggregate(get_handle(), is_aggregate);
    }

    void set_is_stateful(_Bool is_stateful) const noexcept
    {
        m_ops->set_is_stateful(get_handle(), is_stateful);
    }

    void set_allows_uninitialized_input(_Bool allows_uninitialized_input) const noexcept
    {
        m_ops->set_allows_uninitialized_input(get_handle(), allows_uninitialized_input);
    }

    void deprecated(int version, const ice::sonic::String& explanation) const noexcept
    {
        m_ops->deprecated(get_handle(), version, explanation.get_handle());
    }

    void set_shape_inference_function(
        void (*shape_inference_func)(TF_ShapeInferenceContext*, TF_Status*)
    ) const noexcept
    {
        m_ops->set_shape_inference_function(get_handle(), shape_inference_func);
    }

    void register_op_definition(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->register_op_definition(get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
