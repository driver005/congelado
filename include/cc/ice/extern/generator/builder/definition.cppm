// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/definition.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/definition.h"

export module cc_ice_extern_generator_builder:definition;

import std;

export namespace ice::builder {

class TFGeneratorDefinitionOps
{
public:
    TFGeneratorDefinitionOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFGeneratorDefinitionOps(const TFGeneratorDefinitionOps&) = delete;
    TFGeneratorDefinitionOps& operator=(const TFGeneratorDefinitionOps&) = delete;

    static TFGeneratorDefinitionOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorDefinitionOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorDefinitionOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorDefinitionOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorDefinitionOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void set_name(const ice::sonic::String& name) noexcept = 0;
    virtual void set_summary(const ice::sonic::String& summary) noexcept = 0;
    virtual void set_description(const ice::sonic::String& description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    add_input(const ice::sonic::TFGeneratorParameterOps& input) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    add_output(const ice::sonic::TFGeneratorParameterOps& output) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    add_attr(const ice::sonic::TFGeneratorAttributeOps& attr) noexcept = 0;
    virtual void get_summary(const ice::sonic::String& out_summary) noexcept = 0;
    virtual void get_description(const ice::sonic::String& out_description) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    list_inputs(TF_Tensor** out_inputs) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    list_outputs(TF_Tensor** out_outputs) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    list_attrs(TF_Tensor** out_attrs) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGeneratorDefinitionOps{
            .struct_size = TF_ENERATORDEFINITION_STRUCT_SIZE,
            .destroy =
                [](TFGeneratorDefinition* def_context) noexcept
            {
                TFGeneratorDefinitionOps::from_handle(def_context).destroy();
            },
            .get_name =
                [](TFGeneratorDefinition* def_context, TF_String* out_name) noexcept
            {
                TFGeneratorDefinitionOps::from_handle(def_context)
                    .get_name(ice::sonic::String::wrap(out_name));
            },
            .set_name =
                [](TFGeneratorDefinition* def_context, const TF_String* name) noexcept
            {
                TFGeneratorDefinitionOps::from_handle(def_context)
                    .set_name(ice::sonic::String::wrap(name));
            },
            .set_summary =
                [](TFGeneratorDefinition* def_context, const TF_String* summary) noexcept
            {
                TFGeneratorDefinitionOps::from_handle(def_context)
                    .set_summary(ice::sonic::String::wrap(summary));
            },
            .set_description =
                [](TFGeneratorDefinition* def_context, const TF_String* description) noexcept
            {
                TFGeneratorDefinitionOps::from_handle(def_context)
                    .set_description(ice::sonic::String::wrap(description));
            },
            .add_input =
                [](TFGeneratorDefinition* def_context,
                   TFGeneratorParameter* input,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorDefinitionOps::from_handle(def_context)
                               .add_input(ice::sonic::TFGeneratorParameterOps::wrap(input));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_output =
                [](TFGeneratorDefinition* def_context,
                   TFGeneratorParameter* output,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorDefinitionOps::from_handle(def_context)
                               .add_output(ice::sonic::TFGeneratorParameterOps::wrap(output));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_attr =
                [](TFGeneratorDefinition* def_context,
                   TFGeneratorAttribute* attr,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorDefinitionOps::from_handle(def_context)
                               .add_attr(ice::sonic::TFGeneratorAttributeOps::wrap(attr));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_summary =
                [](TFGeneratorDefinition* def_context, TF_String* out_summary) noexcept
            {
                TFGeneratorDefinitionOps::from_handle(def_context)
                    .get_summary(ice::sonic::String::wrap(out_summary));
            },
            .get_description =
                [](TFGeneratorDefinition* def_context, TF_String* out_description) noexcept
            {
                TFGeneratorDefinitionOps::from_handle(def_context)
                    .get_description(ice::sonic::String::wrap(out_description));
            },
            .list_inputs =
                [](TFGeneratorDefinition* def_context,
                   TF_Tensor** out_inputs,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFGeneratorDefinitionOps::from_handle(def_context).list_inputs(out_inputs);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_outputs =
                [](TFGeneratorDefinition* def_context,
                   TF_Tensor** out_outputs,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFGeneratorDefinitionOps::from_handle(def_context).list_outputs(out_outputs);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_attrs =
                [](TFGeneratorDefinition* def_context,
                   TF_Tensor** out_attrs,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorDefinitionOps::from_handle(def_context).list_attrs(out_attrs);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFGeneratorDefinitionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFGeneratorDefinition& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGeneratorDefinitionOps m_vtable;
    TFGeneratorDefinition m_handle;
};

} // namespace ice::builder
