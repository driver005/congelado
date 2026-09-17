// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/function.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/function.h"

export module cc_abi_sonic_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGeneratorFunctionOps :
    public ice::sonic::Runtime<TFGeneratorFunctionOps, TFGeneratorFunctionOps>
{
public:
    explicit TFGeneratorFunctionOps(TFGeneratorFunctionOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    [[nodiscard]] std::expected<void, ice::Status>
    add_parameter(const ice::sonic::TFGeneratorParameterOps& parameter) noexcept
    {
        ice::Status status;
        m_ops->add_parameter(get_handle(), parameter.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_attribute(const ice::sonic::TFGeneratorAttributeOps& attribute) noexcept
    {
        ice::Status status;
        m_ops->add_attribute(get_handle(), attribute.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_definition(const ice::sonic::TFGeneratorDefinitionOps& definition) noexcept
    {
        ice::Status status;
        m_ops->add_definition(get_handle(), definition.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_block(const ice::sonic::TFGeneratorBlockOps& block) noexcept
    {
        ice::Status status;
        m_ops->add_block(get_handle(), block.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_parameter(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFGeneratorParameterOps& out_parameter
    ) noexcept
    {
        ice::Status status;
        m_ops->get_parameter(
            get_handle(),
            name.get_handle(),
            out_parameter.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_attribute(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFGeneratorAttributeOps& out_attribute
    ) noexcept
    {
        ice::Status status;
        m_ops->get_attribute(
            get_handle(),
            name.get_handle(),
            out_attribute.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_definition(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFGeneratorDefinitionOps& out_definition
    ) noexcept
    {
        ice::Status status;
        m_ops->get_definition(
            get_handle(),
            name.get_handle(),
            out_definition.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_block(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFGeneratorBlockOps& out_block
    ) noexcept
    {
        ice::Status status;
        m_ops->get_block(
            get_handle(),
            name.get_handle(),
            out_block.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_parameters(TF_Tensor** out_parameters) noexcept
    {
        ice::Status status;
        m_ops->list_parameters(get_handle(), out_parameters status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_attributes(TF_Tensor** out_attributes) noexcept
    {
        ice::Status status;
        m_ops->list_attributes(get_handle(), out_attributes status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_definitions(TF_Tensor** out_definitions) noexcept
    {
        ice::Status status;
        m_ops->list_definitions(get_handle(), out_definitions status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> list_blocks(TF_Tensor** out_blocks) noexcept
    {
        ice::Status status;
        m_ops->list_blocks(get_handle(), out_blocks status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    finish(const ice::sonic::TF_TensorOps& outputs) noexcept
    {
        ice::Status status;
        m_ops->finish(get_handle(), outputs.get_handle() status.get_handle());

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
