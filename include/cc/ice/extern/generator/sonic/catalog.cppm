// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/catalog.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/catalog.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_generator_sonic:catalog;

import std;
import :module_;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class GeneratorCatalog : public ice::sonic::Runtime<::TFGeneratorCatalogOps, ::TFGeneratorCatalog>
{
public:
    GeneratorCatalog(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    GeneratorCatalog(
        const ice::sonic::Registration& registry,
        const ::TFGeneratorCatalog& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit GeneratorCatalog(const ::TFGeneratorCatalogOps* ops) noexcept :
        Runtime(ops)
    {
    }

    GeneratorCatalog(
        const ::TFGeneratorCatalogOps* ops,
        const ::TFGeneratorCatalog* handle
    ) noexcept :
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
        const ice::sonic::GeneratorModule& module,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_module(get_handle(), module.get_handle(), out_status.get_handle());
    }

    void get_module(
        const ice::sonic::String& name,
        const ice::sonic::GeneratorModule& out_module,
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

    void list_modules(
        ::TF_Tensor** out_modules,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list_modules(get_handle(), out_modules, out_status.get_handle());
    }
};

} // namespace ice::sonic
