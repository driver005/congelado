// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/generator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/generator.h"

export module cc_abi_builder_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_GeneratorOps
{
public:
    static TF_GeneratorOps* create(void* ctx) noexcept
    {
        return static_cast<TF_GeneratorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_GeneratorOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_GeneratorOps*>(handle->plugin_data);
    }

    virtual ~TF_GeneratorOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_module(const ice::sonic::TFGeneratorModuleOps& module) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_module(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFGeneratorModuleOps& out_module
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_modules(TF_Tensor** out_modules) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_name(const ice::sonic::TF_StringOps& name) noexcept = 0;

    static TF_GeneratorOps* get_generic_vtable()
    {
        static TF_GeneratorOps vtable = {
            .struct_size = TF_GENERATOR_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_GeneratorOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_GeneratorOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .add_module =
                [](TF_Generator* generator,
                   TFGeneratorModule* module,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_GeneratorOps::create(generator);
                auto res = self->add_module(ice::sonic::TFGeneratorModuleOps::wrap(module));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_module =
                [](TF_Generator* generator,
                   const TF_String* name,
                   TFGeneratorModule* out_module,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_GeneratorOps::create(generator);
                auto res = self->get_module(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TFGeneratorModuleOps::wrap(out_module)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_modules =
                [](TF_Generator* generator, TF_Tensor** out_modules, TF_Status* out_status) noexcept
            {
                auto* self = TF_GeneratorOps::create(generator);
                auto res = self->list_modules(out_modules);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_name =
                [](TF_Generator* generator, const TF_String* name) noexcept
            {
                auto* self = TF_GeneratorOps::create(generator);
                auto res = self->set_name(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
