// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/parameter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/parameter.h"

export module cc_ice_extern_generator_builder:parameter;

import std;

export namespace ice::builder {

class TFGeneratorParameterOps
{
public:
    TFGeneratorParameterOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFGeneratorParameterOps(const TFGeneratorParameterOps&) = delete;
    TFGeneratorParameterOps& operator=(const TFGeneratorParameterOps&) = delete;

    static TFGeneratorParameterOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorParameterOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorParameterOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorParameterOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorParameterOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_name(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_description(const ice::sonic::String& description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_position(int position) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_description(const ice::sonic::String& out_description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_position(int* out_position) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_type(const ice::sonic::TF_TypeInfoOps& out_type) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGeneratorParameterOps{
            .struct_size = TF_ENERATORPARAMETER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TFGeneratorParameterOps>{
                    &TFGeneratorParameterOps::from_handle(plugin_context)
                };
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TFGeneratorParameterOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .set_name =
                [](TFGeneratorParameter* param_context, const TF_String* name) noexcept
            {
                auto res = TFGeneratorParameterOps::from_handle(param_context)
                               .set_name(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_description =
                [](TFGeneratorParameter* param_context, const TF_String* description) noexcept
            {
                auto res = TFGeneratorParameterOps::from_handle(param_context)
                               .set_description(ice::sonic::String::wrap(description));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_position =
                [](TFGeneratorParameter* param_context, int position) noexcept
            {
                auto res =
                    TFGeneratorParameterOps::from_handle(param_context).set_position(position);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_description =
                [](TFGeneratorParameter* param_context, TF_String* out_description) noexcept
            {
                auto res = TFGeneratorParameterOps::from_handle(param_context)
                               .get_description(ice::sonic::String::wrap(out_description));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_position =
                [](TFGeneratorParameter* param_context, int* out_position) noexcept
            {
                auto res =
                    TFGeneratorParameterOps::from_handle(param_context).get_position(out_position);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_type =
                [](TFGeneratorParameter* param_context, TF_TypeInfo* out_type) noexcept
            {
                auto res = TFGeneratorParameterOps::from_handle(param_context)
                               .get_type(ice::sonic::TF_TypeInfoOps::wrap(out_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TFGeneratorParameterOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TFGeneratorParameter& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGeneratorParameterOps m_vtable;
    TFGeneratorParameter m_handle;
};

} // namespace ice::builder
