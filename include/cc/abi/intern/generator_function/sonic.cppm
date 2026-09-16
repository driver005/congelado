// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/generator_function/generator_function.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/generator_function/generator_function.h"

export module cc_abi_sonic_generator_function;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Generator_function : public ice::sonic::Runtime<Generator_function, TF_Generator_Function>
{
public:
    explicit Generator_function(TF_Generator_Function* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator_function";

    [[nodiscard]] std::expected<void, ice::Status> destroy_function() noexcept
    {
        ice::Status status;
        m_ops->destroy_function(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_parameter(const ice::sonic::String& name, const ice::sonic::String& type_text) noexcept
    {
        ice::Status status;
        m_ops->add_parameter(
            get_handle(),
            name.get_handle(),
            type_text.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> add_node(
        const TF_Generator_Definition* def_context,
        const ice::sonic::Tensor& operands,
        const ice::sonic::Tensor& attrs,
        const ice::sonic::Tensor& out_results
    ) noexcept
    {
        ice::Status status;
        m_ops->add_node(
            get_handle(),
            def_context,
            operands.get_handle(),
            attrs.get_handle(),
            out_results.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    finish(const ice::sonic::Tensor& outputs) noexcept
    {
        ice::Status status;
        m_ops->finish(get_handle(), outputs.get_handle(), status.get_handle());

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
