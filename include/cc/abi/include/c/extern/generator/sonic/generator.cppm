// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/generator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/generator.h"

export module cc_abi_sonic_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_GeneratorOps : public ice::sonic::Runtime<TF_GeneratorOps, TF_GeneratorOps>
{
public:
    explicit TF_GeneratorOps(TF_GeneratorOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    [[nodiscard]] std::expected<void, ice::Status>
    add_module(const ice::sonic::TFGeneratorModuleOps& module) noexcept
    {
        ice::Status status;
        m_ops->add_module(get_handle(), module.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_module(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFGeneratorModuleOps& out_module
    ) noexcept
    {
        ice::Status status;
        m_ops->get_module(
            get_handle(),
            name.get_handle(),
            out_module.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> list_modules(TF_Tensor** out_modules) noexcept
    {
        ice::Status status;
        m_ops->list_modules(get_handle(), out_modules, status.get_handle());

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

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
