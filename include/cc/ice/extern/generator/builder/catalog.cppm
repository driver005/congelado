// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/catalog.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/catalog.h"
#include "include/c/extern/generator/module.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_generator_builder:catalog;

import std;
import cc_ice_extern_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class GeneratorCatalog
{
public:
    explicit GeneratorCatalog(
        const ::TFGeneratorModuleOps* GeneratorModule_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_GeneratorModule_ops = GeneratorModule_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    GeneratorCatalog(const GeneratorCatalog&) = delete;
    GeneratorCatalog& operator=(const GeneratorCatalog&) = delete;

    static GeneratorCatalog& from_handle(void* ctx) noexcept
    {
        return *static_cast<GeneratorCatalog*>(ctx);
    }

    template<typename HandleT>
    static GeneratorCatalog& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<GeneratorCatalog*>(handle->plugin_data);
    }

    virtual ~GeneratorCatalog() = default;
    virtual void destroy() noexcept = 0;
    virtual void add_module(
        const ice::sonic::GeneratorModule& module,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_module(
        const ice::sonic::String& name,
        const ice::sonic::GeneratorModule& out_module,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    list_modules(::TF_Tensor** out_modules, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGeneratorCatalog*)) noexcept
    {
        m_vtable = ::TFGeneratorCatalogOps{
            .struct_size = TF_OFFSET_OF_END(::TFGeneratorCatalogOps, list_modules),

            .create = create,
            .destroy =
                [](::TFGeneratorCatalog* handle) noexcept
            {
                auto& self = GeneratorCatalog::from_handle(handle);
                self.destroy();
            },
            .add_module =
                [](::TFGeneratorCatalog* manager,
                   ::TFGeneratorModule* module,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorCatalog::from_handle(manager);
                self.add_module(
                    self.wrap(std::type_identity<ice::sonic::GeneratorModule>{}, module),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_module =
                [](::TFGeneratorCatalog* manager,
                   const ::TF_String* name,
                   ::TFGeneratorModule* out_module,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorCatalog::from_handle(manager);
                self.get_module(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::GeneratorModule>{}, out_module),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_modules =
                [](::TFGeneratorCatalog* manager,
                   ::TF_Tensor** out_modules,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorCatalog::from_handle(manager);
                self.list_modules(
                    out_modules,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::GeneratorModule wrap(
        std::type_identity<ice::sonic::GeneratorModule>,
        const ::TFGeneratorModule* handle
    ) const noexcept
    {
        return ice::sonic::GeneratorModule{m_GeneratorModule_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TFGeneratorCatalogOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGeneratorCatalog& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFGeneratorCatalogOps*>(&m_vtable));
    }

private:
    ::TFGeneratorCatalogOps m_vtable;
    ::TFGeneratorCatalog m_handle;

    const ::TFGeneratorModuleOps* m_GeneratorModule_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
