// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/catalog.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/catalog.h"
#include "include/c/extern/generator/module.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_generator_builder:catalog;

import std;
import cc_ice_extern_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGeneratorCatalogOps
{
public:
    explicit TFGeneratorCatalogOps(
        const ::TFGeneratorModuleOps* TFGeneratorModuleOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFGeneratorModuleOps_ops = TFGeneratorModuleOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFGeneratorCatalogOps(const TFGeneratorCatalogOps&) = delete;
    TFGeneratorCatalogOps& operator=(const TFGeneratorCatalogOps&) = delete;

    static TFGeneratorCatalogOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorCatalogOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorCatalogOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorCatalogOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorCatalogOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void add_module(
        const ice::sonic::TFGeneratorModuleOps& module,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_module(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorModuleOps& out_module,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    list_modules(TF_Tensor** out_modules, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGeneratorCatalog*)) noexcept
    {
        m_vtable = ::TFGeneratorCatalogOps{
            .struct_size = TF_OFFSET_OF_END(::TFGeneratorCatalogOps, list_modules),

            .create = create,
            .destroy =
                [](TFGeneratorCatalog* handle) noexcept
            {
                auto& self = TFGeneratorCatalogOps::from_handle(handle);
                self.destroy();
            },
            .add_module =
                [](TFGeneratorCatalog* manager,
                   TFGeneratorModule* module,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorCatalogOps::from_handle(manager);
                self.add_module(
                    self.wrap(std::type_identity<ice::sonic::TFGeneratorModuleOps>{}, module),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_module =
                [](TFGeneratorCatalog* manager,
                   const TF_String* name,
                   TFGeneratorModule* out_module,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorCatalogOps::from_handle(manager);
                self.get_module(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::TFGeneratorModuleOps>{}, out_module),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_modules =
                [](TFGeneratorCatalog* manager,
                   TF_Tensor** out_modules,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorCatalogOps::from_handle(manager);
                self.list_modules(
                    out_modules,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TFGeneratorModuleOps wrap(
        std::type_identity<ice::sonic::TFGeneratorModuleOps>,
        const ::TFGeneratorModule* handle
    ) const noexcept
    {
        return ice::sonic::TFGeneratorModuleOps{
            m_TFGeneratorModuleOps_ops,
            const_cast<::TFGeneratorModule*>(handle)
        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TFGeneratorCatalogOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGeneratorCatalog& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFGeneratorCatalogOps*>(&m_vtable));
    }

private:
    ::TFGeneratorCatalogOps m_vtable;
    ::TFGeneratorCatalog m_handle;

    const ::TFGeneratorModuleOps* m_TFGeneratorModuleOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
