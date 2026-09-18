// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/parameter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/parameter.h"

export module cc_ice_builder_generator:parameter;

import std;

export namespace ice::builder {

class TFGeneratorParameterOps
{
public:
    static TFGeneratorParameterOps* create(void* ctx) noexcept
    {
        return static_cast<TFGeneratorParameterOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorParameterOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGeneratorParameterOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorParameterOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_name(const ice::sonic::TF_StringOps& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_description(const ice::sonic::TF_StringOps& description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_position(int position) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_description(const ice::sonic::TF_StringOps& out_description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_position(int* out_position) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_type(const ice::sonic::TF_TypeInfoOps& out_type) noexcept = 0;

    static TFGeneratorParameterOps* get_generic_vtable()
    {
        static TFGeneratorParameterOps vtable = {
            .struct_size = TF_ENERATORPARAMETER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFGeneratorParameterOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFGeneratorParameterOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .set_name =
                [](TFGeneratorParameter* param_context, const TF_String* name) noexcept
            {
                auto* self = TFGeneratorParameterOps::create(param_context);
                auto res = self->set_name(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_description =
                [](TFGeneratorParameter* param_context, const TF_String* description) noexcept
            {
                auto* self = TFGeneratorParameterOps::create(param_context);
                auto res = self->set_description(ice::sonic::TF_StringOps::wrap(description));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_position =
                [](TFGeneratorParameter* param_context, int position) noexcept
            {
                auto* self = TFGeneratorParameterOps::create(param_context);
                auto res = self->set_position(position);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_description =
                [](TFGeneratorParameter* param_context, TF_String* out_description) noexcept
            {
                auto* self = TFGeneratorParameterOps::create(param_context);
                auto res = self->get_description(ice::sonic::TF_StringOps::wrap(out_description));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_position =
                [](TFGeneratorParameter* param_context, int* out_position) noexcept
            {
                auto* self = TFGeneratorParameterOps::create(param_context);
                auto res = self->get_position(out_position);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_type =
                [](TFGeneratorParameter* param_context, TF_TypeInfo* out_type) noexcept
            {
                auto* self = TFGeneratorParameterOps::create(param_context);
                auto res = self->get_type(ice::sonic::TF_TypeInfoOps::wrap(out_type));
                if (!res) {
                    res.error().to_c(status);
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
