// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/function.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/attribute.h"
#include "include/c/extern/generator/block.h"
#include "include/c/extern/generator/definition.h"
#include "include/c/extern/generator/function.h"
#include "include/c/extern/generator/parameter.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_generator_builder:function;

import std;
import cc_ice_extern_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGeneratorFunctionOps
{
public:
    explicit TFGeneratorFunctionOps(
        const ::TFGeneratorAttributeOps* TFGeneratorAttributeOps_ops,
        const ::TFGeneratorBlockOps* TFGeneratorBlockOps_ops,
        const ::TFGeneratorDefinitionOps* TFGeneratorDefinitionOps_ops,
        const ::TFGeneratorParameterOps* TFGeneratorParameterOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_TensorOps* TF_TensorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFGeneratorAttributeOps_ops = TFGeneratorAttributeOps_ops;
        m_TFGeneratorBlockOps_ops = TFGeneratorBlockOps_ops;
        m_TFGeneratorDefinitionOps_ops = TFGeneratorDefinitionOps_ops;
        m_TFGeneratorParameterOps_ops = TFGeneratorParameterOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_TF_TensorOps_ops = TF_TensorOps_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void add_parameter(
        const ice::sonic::TFGeneratorParameterOps& parameter,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void add_attribute(
        const ice::sonic::TFGeneratorAttributeOps& attribute,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void add_definition(
        const ice::sonic::TFGeneratorDefinitionOps& definition,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void add_block(
        const ice::sonic::TFGeneratorBlockOps& block,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_parameter(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorParameterOps& out_parameter,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attribute(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorAttributeOps& out_attribute,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_definition(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorDefinitionOps& out_definition,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_block(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorBlockOps& out_block,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    list_parameters(TF_Tensor** out_parameters, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    list_attributes(TF_Tensor** out_attributes, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void list_definitions(
        TF_Tensor** out_definitions,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    list_blocks(TF_Tensor** out_blocks, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void finish(
        const ice::sonic::TF_TensorOps& outputs,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGeneratorFunction*)) noexcept
    {
        m_vtable = ::TFGeneratorFunctionOps{
            .struct_size = TF_OFFSET_OF_END(::TFGeneratorFunctionOps, finish),

            .create = create,
            .destroy =
                [](TFGeneratorFunction* handle) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFGeneratorFunction* function, TF_String* out_name) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .add_parameter =
                [](TFGeneratorFunction* function,
                   TFGeneratorParameter* parameter,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.add_parameter(
                    self.wrap(std::type_identity<ice::sonic::TFGeneratorParameterOps>{}, parameter),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .add_attribute =
                [](TFGeneratorFunction* function,
                   TFGeneratorAttribute* attribute,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.add_attribute(
                    self.wrap(std::type_identity<ice::sonic::TFGeneratorAttributeOps>{}, attribute),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .add_definition =
                [](TFGeneratorFunction* function,
                   TFGeneratorDefinition* definition,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.add_definition(
                    self.wrap(
                        std::type_identity<ice::sonic::TFGeneratorDefinitionOps>{},
                        definition
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .add_block =
                [](TFGeneratorFunction* function,
                   TFGeneratorBlock* block,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.add_block(
                    self.wrap(std::type_identity<ice::sonic::TFGeneratorBlockOps>{}, block),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_parameter =
                [](TFGeneratorFunction* function,
                   const TF_String* name,
                   TFGeneratorParameter* out_parameter,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.get_parameter(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(
                        std::type_identity<ice::sonic::TFGeneratorParameterOps>{},
                        out_parameter
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attribute =
                [](TFGeneratorFunction* function,
                   const TF_String* name,
                   TFGeneratorAttribute* out_attribute,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.get_attribute(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(
                        std::type_identity<ice::sonic::TFGeneratorAttributeOps>{},
                        out_attribute
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_definition =
                [](TFGeneratorFunction* function,
                   const TF_String* name,
                   TFGeneratorDefinition* out_definition,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.get_definition(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(
                        std::type_identity<ice::sonic::TFGeneratorDefinitionOps>{},
                        out_definition
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_block =
                [](TFGeneratorFunction* function,
                   const TF_String* name,
                   TFGeneratorBlock* out_block,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.get_block(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::TFGeneratorBlockOps>{}, out_block),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_parameters =
                [](TFGeneratorFunction* function,
                   TF_Tensor** out_parameters,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.list_parameters(
                    out_parameters,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_attributes =
                [](TFGeneratorFunction* function,
                   TF_Tensor** out_attributes,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.list_attributes(
                    out_attributes,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_definitions =
                [](TFGeneratorFunction* function,
                   TF_Tensor** out_definitions,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.list_definitions(
                    out_definitions,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_blocks =
                [](TFGeneratorFunction* function,
                   TF_Tensor** out_blocks,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.list_blocks(
                    out_blocks,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .finish =
                [](TFGeneratorFunction* function,
                   const TF_Tensor* outputs,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorFunctionOps::from_handle(function);
                self.finish(
                    self.wrap(std::type_identity<ice::sonic::TF_TensorOps>{}, outputs),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TFGeneratorAttributeOps wrap(
        std::type_identity<ice::sonic::TFGeneratorAttributeOps>,
        const ::TFGeneratorAttribute* handle
    ) const noexcept
    {
        return ice::sonic::TFGeneratorAttributeOps{
            m_TFGeneratorAttributeOps_ops,
            const_cast<::TFGeneratorAttribute*>(handle)
        };
    }

    ice::sonic::TFGeneratorBlockOps wrap(
        std::type_identity<ice::sonic::TFGeneratorBlockOps>,
        const ::TFGeneratorBlock* handle
    ) const noexcept
    {
        return ice::sonic::TFGeneratorBlockOps{
            m_TFGeneratorBlockOps_ops,
            const_cast<::TFGeneratorBlock*>(handle)
        };
    }

    ice::sonic::TFGeneratorDefinitionOps wrap(
        std::type_identity<ice::sonic::TFGeneratorDefinitionOps>,
        const ::TFGeneratorDefinition* handle
    ) const noexcept
    {
        return ice::sonic::TFGeneratorDefinitionOps{
            m_TFGeneratorDefinitionOps_ops,
            const_cast<::TFGeneratorDefinition*>(handle)
        };
    }

    ice::sonic::TFGeneratorParameterOps wrap(
        std::type_identity<ice::sonic::TFGeneratorParameterOps>,
        const ::TFGeneratorParameter* handle
    ) const noexcept
    {
        return ice::sonic::TFGeneratorParameterOps{
            m_TFGeneratorParameterOps_ops,
            const_cast<::TFGeneratorParameter*>(handle)
        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    ice::sonic::TF_TensorOps
    wrap(std::type_identity<ice::sonic::TF_TensorOps>, const ::TF_Tensor* handle) const noexcept
    {
        return ice::sonic::TF_TensorOps{m_TF_TensorOps_ops, const_cast<::TF_Tensor*>(handle)};
    }

    const ::TFGeneratorFunctionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGeneratorFunction& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TFGeneratorFunctionOps*>(&m_vtable)
        );
    }

private:
    ::TFGeneratorFunctionOps m_vtable;
    ::TFGeneratorFunction m_handle;

    const ::TFGeneratorAttributeOps* m_TFGeneratorAttributeOps_ops{nullptr};

    const ::TFGeneratorBlockOps* m_TFGeneratorBlockOps_ops{nullptr};

    const ::TFGeneratorDefinitionOps* m_TFGeneratorDefinitionOps_ops{nullptr};

    const ::TFGeneratorParameterOps* m_TFGeneratorParameterOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_TensorOps* m_TF_TensorOps_ops{nullptr};
};

} // namespace ice::builder
