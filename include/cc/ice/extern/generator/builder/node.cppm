// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/node.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/definition.h"
#include "include/c/extern/generator/node.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_generator_builder:node;

import std;
import cc_ice_extern_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGeneratorNodeOps
{
public:
    explicit TFGeneratorNodeOps(
        const ::TFGeneratorDefinitionOps* TFGeneratorDefinitionOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFGeneratorDefinitionOps_ops = TFGeneratorDefinitionOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TFGeneratorNodeOps(const TFGeneratorNodeOps&) = delete;
    TFGeneratorNodeOps& operator=(const TFGeneratorNodeOps&) = delete;

    static TFGeneratorNodeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGeneratorNodeOps*>(ctx);
    }

    template<typename HandleT>
    static TFGeneratorNodeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGeneratorNodeOps*>(handle->plugin_data);
    }

    virtual ~TFGeneratorNodeOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void set_operand(
        int index,
        const ice::sonic::String& var_name,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_output_name(
        int index,
        const ice::sonic::String& var_name,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_attr(
        const ice::sonic::String& name,
        const void* value,
        size_t value_size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_operand(int index, const ice::sonic::String& out_operand) noexcept = 0;
    virtual void get_output_name(int index, const ice::sonic::String& out_output_name) noexcept = 0;
    virtual void get_definition(
        const ice::sonic::TFGeneratorDefinitionOps& out_definition,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGeneratorNode*)) noexcept
    {
        m_vtable = ::TFGeneratorNodeOps{
            .struct_size = TF_OFFSET_OF_END(::TFGeneratorNodeOps, get_definition),

            .create = create,
            .destroy =
                [](TFGeneratorNode* handle) noexcept
            {
                auto& self = TFGeneratorNodeOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TFGeneratorNode* node_context, TF_String* out_name) noexcept
            {
                auto& self = TFGeneratorNodeOps::from_handle(node_context);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .set_operand =
                [](TFGeneratorNode* node_context,
                   int index,
                   const TF_String* var_name,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorNodeOps::from_handle(node_context);
                self.set_operand(
                    index,
                    self.wrap(std::type_identity<ice::sonic::String>{}, var_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_output_name =
                [](TFGeneratorNode* node_context,
                   int index,
                   const TF_String* var_name,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorNodeOps::from_handle(node_context);
                self.set_output_name(
                    index,
                    self.wrap(std::type_identity<ice::sonic::String>{}, var_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_attr =
                [](TFGeneratorNode* node_context,
                   const TF_String* name,
                   const void* value,
                   size_t value_size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorNodeOps::from_handle(node_context);
                self.set_attr(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    value,
                    value_size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_operand =
                [](TFGeneratorNode* node_context, int index, TF_String* out_operand) noexcept
            {
                auto& self = TFGeneratorNodeOps::from_handle(node_context);
                self.get_operand(
                    index,
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_operand)
                );
            },
            .get_output_name =
                [](TFGeneratorNode* node_context, int index, TF_String* out_output_name) noexcept
            {
                auto& self = TFGeneratorNodeOps::from_handle(node_context);
                self.get_output_name(
                    index,
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_output_name)
                );
            },
            .get_definition =
                [](TFGeneratorNode* node_context,
                   TFGeneratorDefinition* out_definition,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGeneratorNodeOps::from_handle(node_context);
                self.get_definition(
                    self.wrap(
                        std::type_identity<ice::sonic::TFGeneratorDefinitionOps>{},
                        out_definition
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

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

    const ::TFGeneratorNodeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGeneratorNode& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFGeneratorNodeOps*>(&m_vtable));
    }

private:
    ::TFGeneratorNodeOps m_vtable;
    ::TFGeneratorNode m_handle;

    const ::TFGeneratorDefinitionOps* m_TFGeneratorDefinitionOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
