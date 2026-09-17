// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/module.h"

export module cc_abi_builder_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFGeneratorModuleOps
{
public:
    static TFGeneratorModuleOps* create(void* ctx) noexcept
    {
        return static_cast<TFGeneratorModuleOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorModuleOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGeneratorModuleOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorModuleOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    add_function(const ice::sonic::TFGeneratorFunctionOps& function) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_function(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFGeneratorFunctionOps& out_function
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list_functions(TF_Tensor** out_functions) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_name(const ice::sonic::TF_StringOps& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> validate() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    emit(const ice::sonic::TF_StringOps& out_code) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TFGeneratorModuleOps* get_generic_vtable()
    {
        static TFGeneratorModuleOps vtable = {
            .struct_size = TF_ENERATORMODULE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFGeneratorModuleOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFGeneratorModuleOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .add_function =
                [](TFGeneratorModule* module,
                   TFGeneratorFunction* function,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorModuleOps::create(module);
                auto res = self->add_function(ice::sonic::TFGeneratorFunctionOps::wrap(function));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_function =
                [](TFGeneratorModule* module,
                   const TF_String* name,
                   TFGeneratorFunction* out_function,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorModuleOps::create(module);
                auto res = self->get_function(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TFGeneratorFunctionOps::wrap(out_function)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_functions =
                [](TFGeneratorModule* module,
                   TF_Tensor** out_functions,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorModuleOps::create(module);
                auto res = self->list_functions(out_functions);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_name =
                [](TFGeneratorModule* module, const TF_String* name) noexcept
            {
                auto* self = TFGeneratorModuleOps::create(module);
                auto res = self->set_name(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .validate =
                [](TFGeneratorModule* module, TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorModuleOps::create(module);
                auto res = self->validate();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .emit =
                [](TFGeneratorModule* module, TF_String* out_code, TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorModuleOps::create(module);
                auto res = self->emit(ice::sonic::TF_StringOps::wrap(out_code));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
