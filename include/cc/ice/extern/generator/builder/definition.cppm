// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/definition.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/attribute.h"
#include "include/c/extern/generator/definition.h"
#include "include/c/extern/generator/parameter.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_generator_builder:definition;

import std;
import cc_ice_extern_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class GeneratorDefinition
{
public:
    explicit GeneratorDefinition(
        const ::TFGeneratorAttributeOps* GeneratorAttribute_ops,
        const ::TFGeneratorParameterOps* GeneratorParameter_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_GeneratorAttribute_ops = GeneratorAttribute_ops;
        m_GeneratorParameter_ops = GeneratorParameter_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    GeneratorDefinition(const GeneratorDefinition&) = delete;
    GeneratorDefinition& operator=(const GeneratorDefinition&) = delete;

    static GeneratorDefinition& from_handle(void* ctx) noexcept
    {
        return *static_cast<GeneratorDefinition*>(ctx);
    }

    template<typename HandleT>
    static GeneratorDefinition& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<GeneratorDefinition*>(handle->plugin_data);
    }

    virtual ~GeneratorDefinition() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void set_name(const ice::sonic::String& name) noexcept = 0;
    virtual void set_summary(const ice::sonic::String& summary) noexcept = 0;
    virtual void set_description(const ice::sonic::String& description) noexcept = 0;
    virtual void add_input(
        const ice::sonic::GeneratorParameter& input,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void add_output(
        const ice::sonic::GeneratorParameter& output,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void add_attr(
        const ice::sonic::GeneratorAttribute& attr,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_summary(const ice::sonic::String& out_summary) noexcept = 0;
    virtual void get_description(const ice::sonic::String& out_description) noexcept = 0;
    virtual void
    list_inputs(::TF_Tensor** out_inputs, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    list_outputs(::TF_Tensor** out_outputs, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    list_attrs(::TF_Tensor** out_attrs, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGeneratorDefinition*)) noexcept
    {
        m_vtable = ::TFGeneratorDefinitionOps{
            .struct_size = TF_OFFSET_OF_END(::TFGeneratorDefinitionOps, list_attrs),

            .create = create,
            .destroy =
                [](::TFGeneratorDefinition* handle) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TFGeneratorDefinition* def_context, ::TF_String* out_name) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .set_name =
                [](::TFGeneratorDefinition* def_context, const ::TF_String* name) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.set_name(self.wrap(std::type_identity<ice::sonic::String>{}, name));
            },
            .set_summary =
                [](::TFGeneratorDefinition* def_context, const ::TF_String* summary) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.set_summary(self.wrap(std::type_identity<ice::sonic::String>{}, summary));
            },
            .set_description =
                [](::TFGeneratorDefinition* def_context, const ::TF_String* description) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.set_description(
                    self.wrap(std::type_identity<ice::sonic::String>{}, description)
                );
            },
            .add_input =
                [](::TFGeneratorDefinition* def_context,
                   ::TFGeneratorParameter* input,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.add_input(
                    self.wrap(std::type_identity<ice::sonic::GeneratorParameter>{}, input),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .add_output =
                [](::TFGeneratorDefinition* def_context,
                   ::TFGeneratorParameter* output,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.add_output(
                    self.wrap(std::type_identity<ice::sonic::GeneratorParameter>{}, output),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .add_attr =
                [](::TFGeneratorDefinition* def_context,
                   ::TFGeneratorAttribute* attr,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.add_attr(
                    self.wrap(std::type_identity<ice::sonic::GeneratorAttribute>{}, attr),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_summary =
                [](::TFGeneratorDefinition* def_context, ::TF_String* out_summary) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.get_summary(self.wrap(std::type_identity<ice::sonic::String>{}, out_summary));
            },
            .get_description =
                [](::TFGeneratorDefinition* def_context, ::TF_String* out_description) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.get_description(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_description)
                );
            },
            .list_inputs =
                [](::TFGeneratorDefinition* def_context,
                   ::TF_Tensor** out_inputs,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.list_inputs(
                    out_inputs,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_outputs =
                [](::TFGeneratorDefinition* def_context,
                   ::TF_Tensor** out_outputs,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.list_outputs(
                    out_outputs,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_attrs =
                [](::TFGeneratorDefinition* def_context,
                   ::TF_Tensor** out_attrs,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GeneratorDefinition::from_handle(def_context);
                self.list_attrs(
                    out_attrs,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::GeneratorAttribute wrap(
        std::type_identity<ice::sonic::GeneratorAttribute>,
        const ::TFGeneratorAttribute* handle
    ) const noexcept
    {
        return ice::sonic::GeneratorAttribute{m_GeneratorAttribute_ops, handle};
    }

    ice::sonic::GeneratorParameter wrap(
        std::type_identity<ice::sonic::GeneratorParameter>,
        const ::TFGeneratorParameter* handle
    ) const noexcept
    {
        return ice::sonic::GeneratorParameter{m_GeneratorParameter_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TFGeneratorDefinitionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGeneratorDefinition& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFGeneratorDefinitionOps*>(&m_vtable));
    }

private:
    ::TFGeneratorDefinitionOps m_vtable;
    ::TFGeneratorDefinition m_handle;

    const ::TFGeneratorAttributeOps* m_GeneratorAttribute_ops{nullptr};

    const ::TFGeneratorParameterOps* m_GeneratorParameter_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
