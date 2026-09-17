// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/module.h"

export module cc_abi_sonic_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGeneratorModuleOps : public ice::sonic::Runtime<TFGeneratorModuleOps, TFGeneratorModuleOps>
{
public:
    explicit TFGeneratorModuleOps(TFGeneratorModuleOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    [[nodiscard]] std::expected<void, ice::Status>
    add_function(const ice::sonic::TFGeneratorFunctionOps& function) noexcept
    {
        ice::Status status;
        m_ops->add_function(get_handle(), function.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_function(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFGeneratorFunctionOps& out_function
    ) noexcept
    {
        ice::Status status;
        m_ops->get_function(
            get_handle(),
            name.get_handle(),
            out_function.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_functions(TF_Tensor** out_functions) noexcept
    {
        ice::Status status;
        m_ops->list_functions(get_handle(), out_functions, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

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

    [[nodiscard]] std::expected<void, ice::Status> validate() noexcept
    {
        ice::Status status;
        m_ops->validate(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    emit(const ice::sonic::TF_StringOps& out_code) noexcept
    {
        ice::Status status;
        m_ops->emit(get_handle(), out_code.get_handle(), status.get_handle());

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
