// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/module.h"
inline constexpr auto k_struct_size_TFGeneratorModuleOps =
    TF_OFFSET_OF_END(TFGeneratorModuleOps, emit);

export module cc_ice_extern_generator_builder:module_;

import std;
import cc_ice_support;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import c_intern;
import :function;
import c_intern;
import c_intern;
import c_intern;
import :definition;
import c_intern;
import c_intern;
import c_intern;
import :parameter;
import c_intern;
import c_intern;
import :typeinfo;
import c_intern;
import c_intern;
import :attribute;
import c_intern;
import c_intern;
import :parameter;
import :attribute;
import :block;
import c_intern;
import c_intern;
import c_intern;
import :definition;
import :node;
import c_intern;
import c_intern;
import :definition;

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
    virtual void get_name(const ice::sonic::TF_StringOps& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, TF_Status>
    add_function(const ice::sonic::TFGeneratorFunctionOps& function) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, TF_Status> get_function(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TFGeneratorFunctionOps& out_function
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, TF_Status>
    list_functions(TF_Tensor** out_functions) noexcept = 0;
    virtual void set_name(const ice::sonic::TF_StringOps& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, TF_Status> validate() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, TF_Status>
    emit(const ice::sonic::TF_StringOps& out_code) noexcept = 0;

    static ::TFGeneratorModuleOps* get_generic_vtable()
    {
        static ::TFGeneratorModuleOps vtable = {
            .struct_size = k_struct_size_TFGeneratorModuleOps,

            .destroy =
                [](::TFGeneratorModule* handle) noexcept
            {
                delete TFGeneratorModuleOps::create(handle);
            },
            .get_name =
                [](TFGeneratorModule* module, TF_String* out_name) noexcept
            {
                auto* self = TFGeneratorModuleOps::create(module);
                self->get_name(*ice::builder::TF_StringOps::create(out_name));
            },
            .add_function =
                [](TFGeneratorModule* module,
                   TFGeneratorFunction* function,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorModuleOps::create(module);
                auto res =
                    self->add_function(*ice::builder::TFGeneratorFunctionOps::create(function));
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
                    *ice::builder::TF_StringOps::create(name),
                    *ice::builder::TFGeneratorFunctionOps::create(out_function)
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
                self->set_name(*ice::builder::TF_StringOps::create(name));
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
                auto res = self->emit(*ice::builder::TF_StringOps::create(out_code));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
        };

        return &vtable;
    }
};

} // namespace ice::builder
