// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/op_definition_builder.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/op_definition_builder.h"

export module cc_ice_extern_ops_builder:op_definition_builder;

import std;

export namespace ice::builder {

class TF_OpDefinitionBuilderOps
{
public:
    TF_OpDefinitionBuilderOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_attr(const ice::sonic::String& attr_spec) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_input(const ice::sonic::String& input_spec) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_output(const ice::sonic::String& output_spec) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_is_commutative(_Bool is_commutative) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_is_aggregate(_Bool is_aggregate) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_is_stateful(_Bool is_stateful) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_allows_uninitialized_input(_Bool allows_uninitialized_input) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    deprecated(int version, const ice::sonic::String& explanation) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_shape_inference_function(
        void (*)(TF_ShapeInferenceContext*, TF_Status*) shape_inference_func
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> register_op_definition() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_OpDefinitionBuilderOps{
            .struct_size = TF_OPDEFINITIONBUILDER_STRUCT_SIZE,
            .add_attr =
                [](TF_OpDefinitionBuilder* builder, const TF_String* attr_spec) noexcept
            {
                auto res = TF_OpDefinitionBuilderOps::from_handle(builder).add_attr(
                    ice::sonic::String::wrap(attr_spec)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .add_input =
                [](TF_OpDefinitionBuilder* builder, const TF_String* input_spec) noexcept
            {
                auto res = TF_OpDefinitionBuilderOps::from_handle(builder).add_input(
                    ice::sonic::String::wrap(input_spec)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .add_output =
                [](TF_OpDefinitionBuilder* builder, const TF_String* output_spec) noexcept
            {
                auto res = TF_OpDefinitionBuilderOps::from_handle(builder).add_output(
                    ice::sonic::String::wrap(output_spec)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_is_commutative =
                [](TF_OpDefinitionBuilder* builder, _Bool is_commutative) noexcept
            {
                auto res = TF_OpDefinitionBuilderOps::from_handle(builder).set_is_commutative(
                    is_commutative
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_is_aggregate =
                [](TF_OpDefinitionBuilder* builder, _Bool is_aggregate) noexcept
            {
                auto res =
                    TF_OpDefinitionBuilderOps::from_handle(builder).set_is_aggregate(is_aggregate);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_is_stateful =
                [](TF_OpDefinitionBuilder* builder, _Bool is_stateful) noexcept
            {
                auto res =
                    TF_OpDefinitionBuilderOps::from_handle(builder).set_is_stateful(is_stateful);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_allows_uninitialized_input =
                [](TF_OpDefinitionBuilder* builder, _Bool allows_uninitialized_input) noexcept
            {
                auto res =
                    TF_OpDefinitionBuilderOps::from_handle(builder).set_allows_uninitialized_input(
                        allows_uninitialized_input
                    );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .deprecated =
                [](TF_OpDefinitionBuilder* builder,
                   int version,
                   const TF_String* explanation) noexcept
            {
                auto res = TF_OpDefinitionBuilderOps::from_handle(builder).deprecated(
                    version,
                    ice::sonic::String::wrap(explanation)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_shape_inference_function =
                [](TF_OpDefinitionBuilder* builder,
                   void (*)(TF_ShapeInferenceContext*, TF_Status*) shape_inference_func) noexcept
            {
                auto res =
                    TF_OpDefinitionBuilderOps::from_handle(builder).set_shape_inference_function(
                        shape_inference_func
                    );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .register_op_definition =
                [](TF_OpDefinitionBuilder* builder, TF_Status* out_status) noexcept
            {
                auto res = TF_OpDefinitionBuilderOps::from_handle(builder).register_op_definition();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_OpDefinitionBuilderOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_OpDefinitionBuilder& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_OpDefinitionBuilderOps m_vtable;
    TF_OpDefinitionBuilder m_handle;
};

} // namespace ice::builder
