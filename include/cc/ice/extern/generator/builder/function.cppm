// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/function.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/function.h"

export module cc_ice_extern_generator_builder:function;

import std;

export namespace ice::builder {

class TFGeneratorFunctionOps
{
public:
    TFGeneratorFunctionOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFGeneratorFunctionOps(const TFGeneratorFunctionOps&) = delete;
    TFGeneratorFunctionOps& operator=(const TFGeneratorFunctionOps&) = delete;

    static TFGeneratorFunctionOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorFunctionOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorFunctionOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorFunctionOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorFunctionOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_parameter(const ice::sonic::TFGeneratorParameterOps& parameter) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_attribute(const ice::sonic::TFGeneratorAttributeOps& attribute) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_definition(const ice::sonic::TFGeneratorDefinitionOps& definition) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_block(const ice::sonic::TFGeneratorBlockOps& block) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_parameter(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorParameterOps& out_parameter
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_attribute(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorAttributeOps& out_attribute
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_definition(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorDefinitionOps& out_definition
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_block(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorBlockOps& out_block
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_parameters(TF_Tensor** out_parameters) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_attributes(TF_Tensor** out_attributes) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_definitions(TF_Tensor** out_definitions) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list_blocks(TF_Tensor** out_blocks) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    finish(const ice::sonic::TF_TensorOps& outputs) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGeneratorFunctionOps{
            .struct_size = TF_ENERATORFUNCTION_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TFGeneratorFunctionOps>{
                    &TFGeneratorFunctionOps::from_handle(plugin_context)
                };
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TFGeneratorFunctionOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .add_parameter =
                [](TFGeneratorFunction* function,
                   TFGeneratorParameter* parameter,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorFunctionOps::from_handle(function).add_parameter(
                    ice::sonic::TFGeneratorParameterOps::wrap(parameter)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_attribute =
                [](TFGeneratorFunction* function,
                   TFGeneratorAttribute* attribute,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorFunctionOps::from_handle(function).add_attribute(
                    ice::sonic::TFGeneratorAttributeOps::wrap(attribute)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_definition =
                [](TFGeneratorFunction* function,
                   TFGeneratorDefinition* definition,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorFunctionOps::from_handle(function).add_definition(
                    ice::sonic::TFGeneratorDefinitionOps::wrap(definition)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_block =
                [](TFGeneratorFunction* function,
                   TFGeneratorBlock* block,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorFunctionOps::from_handle(function).add_block(
                    ice::sonic::TFGeneratorBlockOps::wrap(block)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_parameter =
                [](TFGeneratorFunction* function,
                   const TF_String* name,
                   TFGeneratorParameter* out_parameter,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorFunctionOps::from_handle(function).get_parameter(
                    ice::sonic::String::wrap(name),
                    ice::sonic::TFGeneratorParameterOps::wrap(out_parameter)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attribute =
                [](TFGeneratorFunction* function,
                   const TF_String* name,
                   TFGeneratorAttribute* out_attribute,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorFunctionOps::from_handle(function).get_attribute(
                    ice::sonic::String::wrap(name),
                    ice::sonic::TFGeneratorAttributeOps::wrap(out_attribute)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_definition =
                [](TFGeneratorFunction* function,
                   const TF_String* name,
                   TFGeneratorDefinition* out_definition,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorFunctionOps::from_handle(function).get_definition(
                    ice::sonic::String::wrap(name),
                    ice::sonic::TFGeneratorDefinitionOps::wrap(out_definition)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_block =
                [](TFGeneratorFunction* function,
                   const TF_String* name,
                   TFGeneratorBlock* out_block,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorFunctionOps::from_handle(function).get_block(
                    ice::sonic::String::wrap(name),
                    ice::sonic::TFGeneratorBlockOps::wrap(out_block)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_parameters =
                [](TFGeneratorFunction* function,
                   TF_Tensor** out_parameters,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFGeneratorFunctionOps::from_handle(function).list_parameters(out_parameters);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_attributes =
                [](TFGeneratorFunction* function,
                   TF_Tensor** out_attributes,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFGeneratorFunctionOps::from_handle(function).list_attributes(out_attributes);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_definitions =
                [](TFGeneratorFunction* function,
                   TF_Tensor** out_definitions,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFGeneratorFunctionOps::from_handle(function).list_definitions(out_definitions);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_blocks =
                [](TFGeneratorFunction* function,
                   TF_Tensor** out_blocks,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorFunctionOps::from_handle(function).list_blocks(out_blocks);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .finish =
                [](TFGeneratorFunction* function,
                   const TF_Tensor* outputs,
                   TF_Status* out_status) noexcept
            {
                auto res = TFGeneratorFunctionOps::from_handle(function).finish(
                    ice::sonic::TF_TensorOps::wrap(outputs)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFGeneratorFunctionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TFGeneratorFunction& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGeneratorFunctionOps m_vtable;
    TFGeneratorFunction m_handle;
};

} // namespace ice::builder
