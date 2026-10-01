// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/op_definition_builder.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/op_definition_builder.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_ops_builder:op_definition_builder;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_OpDefinitionBuilderOps
{
public:
    explicit TF_OpDefinitionBuilderOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TF_OpDefinitionBuilderOps(const TF_OpDefinitionBuilderOps&) = delete;
    TF_OpDefinitionBuilderOps& operator=(const TF_OpDefinitionBuilderOps&) = delete;

    static TF_OpDefinitionBuilderOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_OpDefinitionBuilderOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OpDefinitionBuilderOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_OpDefinitionBuilderOps*>(handle->plugin_data);
    }

    virtual ~TF_OpDefinitionBuilderOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void add_attr(const ice::sonic::String& attr_spec) noexcept = 0;
    virtual void add_input(const ice::sonic::String& input_spec) noexcept = 0;
    virtual void add_output(const ice::sonic::String& output_spec) noexcept = 0;
    virtual void set_is_commutative(_Bool is_commutative) noexcept = 0;
    virtual void set_is_aggregate(_Bool is_aggregate) noexcept = 0;
    virtual void set_is_stateful(_Bool is_stateful) noexcept = 0;
    virtual void set_allows_uninitialized_input(_Bool allows_uninitialized_input) noexcept = 0;
    virtual void deprecated(int version, const ice::sonic::String& explanation) noexcept = 0;
    virtual void set_shape_inference_function(
        void (*shape_inference_func)(TF_ShapeInferenceContext*, TF_Status*)
    ) noexcept = 0;
    virtual void register_op_definition(const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_OpDefinitionBuilder*)) noexcept
    {
        m_vtable = ::TF_OpDefinitionBuilderOps{
            .struct_size = TF_OFFSET_OF_END(::TF_OpDefinitionBuilderOps, register_op_definition),

            .create = create,
            .destroy =
                [](TF_OpDefinitionBuilder* handle) noexcept
            {
                auto& self = TF_OpDefinitionBuilderOps::from_handle(handle);
                self.destroy();
            },
            .add_attr =
                [](TF_OpDefinitionBuilder* builder, const TF_String* attr_spec) noexcept
            {
                auto& self = TF_OpDefinitionBuilderOps::from_handle(builder);
                self.add_attr(self.wrap(std::type_identity<ice::sonic::String>{}, attr_spec));
            },
            .add_input =
                [](TF_OpDefinitionBuilder* builder, const TF_String* input_spec) noexcept
            {
                auto& self = TF_OpDefinitionBuilderOps::from_handle(builder);
                self.add_input(self.wrap(std::type_identity<ice::sonic::String>{}, input_spec));
            },
            .add_output =
                [](TF_OpDefinitionBuilder* builder, const TF_String* output_spec) noexcept
            {
                auto& self = TF_OpDefinitionBuilderOps::from_handle(builder);
                self.add_output(self.wrap(std::type_identity<ice::sonic::String>{}, output_spec));
            },
            .set_is_commutative =
                [](TF_OpDefinitionBuilder* builder, _Bool is_commutative) noexcept
            {
                auto& self = TF_OpDefinitionBuilderOps::from_handle(builder);
                self.set_is_commutative(is_commutative);
            },
            .set_is_aggregate =
                [](TF_OpDefinitionBuilder* builder, _Bool is_aggregate) noexcept
            {
                auto& self = TF_OpDefinitionBuilderOps::from_handle(builder);
                self.set_is_aggregate(is_aggregate);
            },
            .set_is_stateful =
                [](TF_OpDefinitionBuilder* builder, _Bool is_stateful) noexcept
            {
                auto& self = TF_OpDefinitionBuilderOps::from_handle(builder);
                self.set_is_stateful(is_stateful);
            },
            .set_allows_uninitialized_input =
                [](TF_OpDefinitionBuilder* builder, _Bool allows_uninitialized_input) noexcept
            {
                auto& self = TF_OpDefinitionBuilderOps::from_handle(builder);
                self.set_allows_uninitialized_input(allows_uninitialized_input);
            },
            .deprecated =
                [](TF_OpDefinitionBuilder* builder,
                   int version,
                   const TF_String* explanation) noexcept
            {
                auto& self = TF_OpDefinitionBuilderOps::from_handle(builder);
                self.deprecated(
                    version,
                    self.wrap(std::type_identity<ice::sonic::String>{}, explanation)
                );
            },
            .set_shape_inference_function =
                [](TF_OpDefinitionBuilder* builder,
                   void (*shape_inference_func)(TF_ShapeInferenceContext*, TF_Status*)) noexcept
            {
                auto& self = TF_OpDefinitionBuilderOps::from_handle(builder);
                self.set_shape_inference_function(shape_inference_func);
            },
            .register_op_definition =
                [](TF_OpDefinitionBuilder* builder, TF_Status* out_status) noexcept
            {
                auto& self = TF_OpDefinitionBuilderOps::from_handle(builder);
                self.register_op_definition(
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

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

    const ::TF_OpDefinitionBuilderOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_OpDefinitionBuilder& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_OpDefinitionBuilderOps*>(&m_vtable));
    }

private:
    ::TF_OpDefinitionBuilderOps m_vtable;
    ::TF_OpDefinitionBuilder m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
