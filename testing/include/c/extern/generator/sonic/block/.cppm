// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/block.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/block.h"

export module cc_abi_sonic_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGeneratorBlockOps : public ice::sonic::Runtime<TFGeneratorBlockOps, TFGeneratorBlockOps>
{
public:
    explicit TFGeneratorBlockOps(TFGeneratorBlockOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    [[nodiscard]] std::expected<void, ice::Status> add_node(
        const ice::sonic::TFGeneratorDefinitionOps& definition,
        const ice::sonic::TFGeneratorNodeOps& out_node
    ) noexcept
    {
        ice::Status status;
        m_ops->add_node(
            get_handle(),
            definition.get_handle(),
            out_node.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_node(int index, const ice::sonic::TFGeneratorNodeOps& out_node) noexcept
    {
        ice::Status status;
        m_ops->get_node(get_handle(), index, out_node.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> list_nodes(TF_Tensor** out_nodes) noexcept
    {
        ice::Status status;
        m_ops->list_nodes(get_handle(), out_nodes status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_name(const ice::sonic::TF_StringOps& name) noexcept
    {
        ice::Status status;
        m_ops->set_name(get_handle(), name.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
