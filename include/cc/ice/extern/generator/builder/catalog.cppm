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
    static TFGeneratorCatalogOps* create(void* ctx) noexcept
    {
        return static_cast<TFGeneratorCatalogOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorCatalogOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGeneratorCatalogOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorCatalogOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_module(const ice::sonic::TFGeneratorModuleOps& module) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_module(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFGeneratorModuleOps& out_module
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_modules(TF_Tensor** out_modules) noexcept = 0;

    static TFGeneratorCatalogOps* get_generic_vtable()
    {
        static TFGeneratorCatalogOps vtable = {
            .struct_size = TF_ENERATORCATALOG_STRUCT_SIZE,
            .add_module =
                [](TFGeneratorCatalog* manager,
                   TFGeneratorModule* module,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorCatalogOps::create(manager);
                auto res = self->add_module(ice::sonic::TFGeneratorModuleOps::wrap(module));
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
                auto* self = TFGeneratorCatalogOps::create(manager);
                auto res = self->get_module(
                    ice::sonic::TF_StringOps::wrap(name),
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
                auto* self = TFGeneratorCatalogOps::create(manager);
                auto res = self->list_modules(out_modules);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
