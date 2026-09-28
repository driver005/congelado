// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/module.h"

export module cc_ice_extern_generator_sonic:module_;

import std;
import cc_ice_support;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import :function;
import c_intern;
import c_intern;
import c_intern;
import :definition;
import c_intern;
import c_intern;
import c_intern;
import :parameter;
import c_intern;
import c_intern;
import :typeinfo;
import c_intern;
import c_intern;
import :attribute;
import c_intern;
import c_intern;
import :parameter;
import :attribute;
import :block;
import c_intern;
import c_intern;
import c_intern;
import :definition;
import :node;
import c_intern;
import c_intern;
import :definition;

export namespace ice::sonic {

class TFGeneratorModuleOps :
    public ice::sonic::Runtime<TFGeneratorModuleOps, ::TFGeneratorModuleOps, ::TFGeneratorModule>
{
public:
    explicit TFGeneratorModuleOps(::TFGeneratorModuleOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    void get_name(const ice::sonic::TF_StringOps& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    [[nodiscard]] std::expected<void, TF_Status>
    add_function(const ice::sonic::TFGeneratorFunctionOps& function) noexcept
    {
        TF_Status status;
        m_ops->add_function(get_handle(), function.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, TF_Status> get_function(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFGeneratorFunctionOps& out_function
    ) noexcept
    {
        TF_Status status;
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

    [[nodiscard]] std::expected<void, TF_Status> list_functions(TF_Tensor** out_functions) noexcept
    {
        TF_Status status;
        m_ops->list_functions(get_handle(), out_functions, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void set_name(const ice::sonic::TF_StringOps& name) noexcept
    {
        m_ops->set_name(get_handle(), name.get_handle());
    }

    [[nodiscard]] std::expected<void, TF_Status> validate() noexcept
    {
        TF_Status status;
        m_ops->validate(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, TF_Status>
    emit(const ice::sonic::TF_StringOps& out_code) noexcept
    {
        TF_Status status;
        m_ops->emit(get_handle(), out_code.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
