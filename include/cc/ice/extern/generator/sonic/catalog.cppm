// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/catalog.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/catalog.h"

export module cc_ice_extern_generator_sonic:catalog;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGeneratorCatalogOps :
    public ice::sonic::Runtime<TFGeneratorCatalogOps, TFGeneratorCatalogOps>
{
public:
    explicit TFGeneratorCatalogOps(TFGeneratorCatalogOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    add_module(const ice::sonic::TFGeneratorModuleOps& module) noexcept
    {
        ice::sonic::Status status;
        m_ops->add_module(get_handle(), module.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> get_module(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorModuleOps& out_module
    ) noexcept
    {
        ice::sonic::Status status;
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

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    list_modules(TF_Tensor** out_modules) noexcept
    {
        ice::sonic::Status status;
        m_ops->list_modules(get_handle(), out_modules, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
