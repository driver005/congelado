// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/block.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/block.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_generator_sonic:block;

import std;
import :definition;
import :node;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGeneratorBlockOps : public ice::sonic::Runtime<::TFGeneratorBlockOps, ::TFGeneratorBlock>
{
public:
    TFGeneratorBlockOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFGeneratorBlockOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFGeneratorBlock* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFGeneratorBlockOps(const ::TFGeneratorBlockOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGeneratorBlockOps(const ::TFGeneratorBlockOps* ops, ::TFGeneratorBlock* handle) noexcept :
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

    void add_node(
        const ice::sonic::TFGeneratorDefinitionOps& definition,
        const ice::sonic::TFGeneratorNodeOps& out_node,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_node(
            get_handle(),
            definition.get_handle(),
            out_node.get_handle(),
            out_status.get_handle()
        );
    }

    void get_node(
        int index,
        const ice::sonic::TFGeneratorNodeOps& out_node,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_node(get_handle(), index, out_node.get_handle(), out_status.get_handle());
    }

    void list_nodes(TF_Tensor** out_nodes, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->list_nodes(get_handle(), out_nodes, out_status.get_handle());
    }

    void set_name(const ice::sonic::String& name) const noexcept
    {
        m_ops->set_name(get_handle(), name.get_handle());
    }
};

} // namespace ice::sonic
