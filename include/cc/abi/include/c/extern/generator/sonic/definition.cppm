// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/definition.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/definition.h"

export module cc_abi_sonic_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGeneratorDefinitionOps :
    public ice::sonic::Runtime<TFGeneratorDefinitionOps, TFGeneratorDefinitionOps>
{
public:
    explicit TFGeneratorDefinitionOps(TFGeneratorDefinitionOps* ops, void* plugin_context) noexcept
        :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    [[nodiscard]] std::expected<void, ice::Status>
    set_name(const ice::sonic::TF_StringOps& name) noexcept
    {
        ice::Status status;
        m_ops->set_name(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_summary(const ice::sonic::TF_StringOps& summary) noexcept
    {
        ice::Status status;
        m_ops->set_summary(get_handle(), summary.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_description(const ice::sonic::TF_StringOps& description) noexcept
    {
        ice::Status status;
        m_ops->set_description(get_handle(), description.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_input(const ice::sonic::TFGeneratorParameterOps& input) noexcept
    {
        ice::Status status;
        m_ops->add_input(get_handle(), input.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_output(const ice::sonic::TFGeneratorParameterOps& output) noexcept
    {
        ice::Status status;
        m_ops->add_output(get_handle(), output.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_attr(const ice::sonic::TFGeneratorAttributeOps& attr) noexcept
    {
        ice::Status status;
        m_ops->add_attr(get_handle(), attr.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_summary(const ice::sonic::TF_StringOps& out_summary) noexcept
    {
        ice::Status status;
        m_ops->get_summary(get_handle(), out_summary.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_description(const ice::sonic::TF_StringOps& out_description) noexcept
    {
        ice::Status status;
        m_ops->get_description(get_handle(), out_description.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> list_inputs(TF_Tensor** out_inputs) noexcept
    {
        ice::Status status;
        m_ops->list_inputs(get_handle(), out_inputs, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> list_outputs(TF_Tensor** out_outputs) noexcept
    {
        ice::Status status;
        m_ops->list_outputs(get_handle(), out_outputs, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> list_attrs(TF_Tensor** out_attrs) noexcept
    {
        ice::Status status;
        m_ops->list_attrs(get_handle(), out_attrs, status.get_handle());

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
