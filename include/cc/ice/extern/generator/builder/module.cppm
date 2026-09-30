// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/module.h"

export module cc_ice_extern_generator_builder:module;

import std;

export namespace ice::builder {

class TFGeneratorModuleOps
{
public:
    TFGeneratorModuleOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFGeneratorModuleOps(const TFGeneratorModuleOps&) = delete;
    TFGeneratorModuleOps& operator=(const TFGeneratorModuleOps&) = delete;

    static TFGeneratorModuleOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorModuleOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorModuleOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorModuleOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorModuleOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    add_function(const ice::sonic::TFGeneratorFunctionOps& function) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_function(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorFunctionOps& out_function
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    list_functions(TF_Tensor** out_functions) noexcept = 0;
    virtual void set_name(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> validate() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    emit(const ice::sonic::String& out_code) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGeneratorModuleOps{
            .struct_size = TF_ENERATORMODULE_STRUCT_SIZE,
            .destroy =
                [](TFGeneratorModule* module) noexcept
            {
                TFGeneratorModuleOps::from_handle(module).destroy();
            },
            .get_name =
                [](TFGeneratorModule* module, TF_String* out_name) noexcept
            {
                TFGeneratorModuleOps::from_handle(module).get_name(
                    ice::sonic::String::wrap(out_name)
                );
            },
            .add_function =
                [](TFGeneratorModule* module,
                   TFGeneratorFunction* function,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorModuleOps::from_handle(module).add_function(
                    ice::sonic::TFGeneratorFunctionOps::wrap(function)
                );
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
                auto res = TFGeneratorModuleOps::from_handle(module).get_function(
                    ice::sonic::String::wrap(name),
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
                auto res = TFGeneratorModuleOps::from_handle(module).list_functions(out_functions);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_name =
                [](TFGeneratorModule* module, const TF_String* name) noexcept
            {
                TFGeneratorModuleOps::from_handle(module).set_name(ice::sonic::String::wrap(name));
            },
            .validate =
                [](TFGeneratorModule* module, TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorModuleOps::from_handle(module).validate();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .emit =
                [](TFGeneratorModule* module, TF_String* out_code, TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorModuleOps::from_handle(module).emit(
                    ice::sonic::String::wrap(out_code)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFGeneratorModuleOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFGeneratorModule& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGeneratorModuleOps m_vtable;
    TFGeneratorModule m_handle;
};

} // namespace ice::builder
