// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/catalog.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/catalog.h"

export module cc_ice_extern_generator_builder:catalog;

import std;

export namespace ice::builder {

class TFGeneratorCatalogOps
{
public:
    TFGeneratorCatalogOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    add_module(const ice::sonic::TFGeneratorModuleOps& module) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_module(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorModuleOps& out_module
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    list_modules(TF_Tensor** out_modules) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGeneratorCatalogOps{
            .struct_size = TF_ENERATORCATALOG_STRUCT_SIZE,
            .add_module =
                [](TFGeneratorCatalog* manager,
                   TFGeneratorModule* module,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorCatalogOps::from_handle(manager).add_module(
                    ice::sonic::TFGeneratorModuleOps::wrap(module)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_module =
                [](TFGeneratorCatalog* manager,
                   const TF_String* name,
                   TFGeneratorModule* out_module,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorCatalogOps::from_handle(manager).get_module(
                    ice::sonic::String::wrap(name),
                    ice::sonic::TFGeneratorModuleOps::wrap(out_module)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_modules =
                [](TFGeneratorCatalog* manager,
                   TF_Tensor** out_modules,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorCatalogOps::from_handle(manager).list_modules(out_modules);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFGeneratorCatalogOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFGeneratorCatalog& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGeneratorCatalogOps m_vtable;
    TFGeneratorCatalog m_handle;
};

} // namespace ice::builder
