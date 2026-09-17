// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/definition.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/definition.h"

export module cc_abi_builder_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFGeneratorDefinitionOps
{
public:
    static TFGeneratorDefinitionOps* create(void* ctx) noexcept
    {
        return static_cast<TFGeneratorDefinitionOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorDefinitionOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGeneratorDefinitionOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorDefinitionOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    set_name(const ice::sonic::TF_StringOps& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_summary(const ice::sonic::TF_StringOps& summary) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_description(const ice::sonic::TF_StringOps& description) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    add_input(const ice::sonic::TFGeneratorParameterOps& input) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    add_output(const ice::sonic::TFGeneratorParameterOps& output) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    add_attr(const ice::sonic::TFGeneratorAttributeOps& attr) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_summary(const ice::sonic::TF_StringOps& out_summary) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_description(const ice::sonic::TF_StringOps& out_description) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> list_inputs(TF_Tensor** out_inputs) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list_outputs(TF_Tensor** out_outputs) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> list_attrs(TF_Tensor** out_attrs) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TFGeneratorDefinitionOps* get_generic_vtable()
    {
        static TFGeneratorDefinitionOps vtable = {
            .struct_size = TF_ENERATORDEFINITION_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFGeneratorDefinitionOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .set_name =
                [](TFGeneratorDefinition* def_context, const TF_String* name) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(def_context);
                auto res = self->set_name(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_summary =
                [](TFGeneratorDefinition* def_context, const TF_String* summary) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(def_context);
                auto res = self->set_summary(ice::sonic::TF_StringOps::wrap(summary));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_description =
                [](TFGeneratorDefinition* def_context, const TF_String* description) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(def_context);
                auto res = self->set_description(ice::sonic::TF_StringOps::wrap(description));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .add_input =
                [](TFGeneratorDefinition* def_context,
                   TFGeneratorParameter* input,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(def_context);
                auto res = self->add_input(ice::sonic::TFGeneratorParameterOps::wrap(input));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_output =
                [](TFGeneratorDefinition* def_context,
                   TFGeneratorParameter* output,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(def_context);
                auto res = self->add_output(ice::sonic::TFGeneratorParameterOps::wrap(output));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_attr =
                [](TFGeneratorDefinition* def_context,
                   TFGeneratorAttribute* attr,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(def_context);
                auto res = self->add_attr(ice::sonic::TFGeneratorAttributeOps::wrap(attr));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_summary =
                [](TFGeneratorDefinition* def_context, TF_String* out_summary) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(def_context);
                auto res = self->get_summary(ice::sonic::TF_StringOps::wrap(out_summary));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_description =
                [](TFGeneratorDefinition* def_context, TF_String* out_description) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(def_context);
                auto res = self->get_description(ice::sonic::TF_StringOps::wrap(out_description));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_inputs =
                [](TFGeneratorDefinition* def_context,
                   TF_Tensor** out_inputs,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(def_context);
                auto res = self->list_inputs(out_inputs);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_outputs =
                [](TFGeneratorDefinition* def_context,
                   TF_Tensor** out_outputs,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(def_context);
                auto res = self->list_outputs(out_outputs);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_attrs =
                [](TFGeneratorDefinition* def_context,
                   TF_Tensor** out_attrs,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGeneratorDefinitionOps::create(def_context);
                auto res = self->list_attrs(out_attrs);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
