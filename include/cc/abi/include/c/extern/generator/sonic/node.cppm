// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/node.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/node.h"

export module cc_abi_sonic_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGeneratorNodeOps : public ice::sonic::Runtime<TFGeneratorNodeOps, TFGeneratorNodeOps>
{
public:
    explicit TFGeneratorNodeOps(TFGeneratorNodeOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    [[nodiscard]] std::expected<void, ice::Status>
    set_operand(int index, const ice::sonic::TF_StringOps& var_name) noexcept
    {
        ice::Status status;
        m_ops->set_operand(get_handle(), index, var_name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_output_name(int index, const ice::sonic::TF_StringOps& var_name) noexcept
    {
        ice::Status status;
        m_ops->set_output_name(get_handle(), index, var_name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_attr(const ice::sonic::TF_StringOps& name, const void* value, size_t value_size) noexcept
    {
        ice::Status status;
        m_ops->set_attr(get_handle(), name.get_handle(), value, value_size, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_operand(int index, const ice::sonic::TF_StringOps& out_operand) noexcept
    {
        ice::Status status;
        m_ops->get_operand(get_handle(), index, out_operand.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_output_name(int index, const ice::sonic::TF_StringOps& out_output_name) noexcept
    {
        ice::Status status;
        m_ops->get_output_name(
            get_handle(),
            index,
            out_output_name.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_definition(const ice::sonic::TFGeneratorDefinitionOps& out_definition) noexcept
    {
        ice::Status status;
        m_ops->get_definition(get_handle(), out_definition.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
