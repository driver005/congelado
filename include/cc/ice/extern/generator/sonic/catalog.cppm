// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/catalog.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/catalog.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_generator_sonic:catalog;

import std;
import :module_;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGeneratorCatalogOps :
    public ice::sonic::Runtime<::TFGeneratorCatalogOps, ::TFGeneratorCatalog>
{
public:
    TFGeneratorCatalogOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFGeneratorCatalogOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFGeneratorCatalog* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFGeneratorCatalogOps(const ::TFGeneratorCatalogOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGeneratorCatalogOps(const ::TFGeneratorCatalogOps* ops, ::TFGeneratorCatalog* handle) noexcept
        :
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

    void add_module(
        const ice::sonic::TFGeneratorModuleOps& module,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_module(get_handle(), module.get_handle(), out_status.get_handle());
    }

    void get_module(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorModuleOps& out_module,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_module(
            get_handle(),
            name.get_handle(),
            out_module.get_handle(),
            out_status.get_handle()
        );
    }

    void list_modules(TF_Tensor** out_modules, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->list_modules(get_handle(), out_modules, out_status.get_handle());
    }
};

} // namespace ice::sonic
