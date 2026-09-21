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
    static TF_OpDefinitionBuilderOps* create(void* ctx) noexcept
    {
        return static_cast<TF_OpDefinitionBuilderOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OpDefinitionBuilderOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_OpDefinitionBuilderOps*>(handle->plugin_data);
    }

    virtual ~TF_OpDefinitionBuilderOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_attr(const char* attr_spec) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_input(const char* input_spec) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_output(const char* output_spec) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_is_commutative(_Bool is_commutative) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_is_aggregate(_Bool is_aggregate) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_is_stateful(_Bool is_stateful) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_allows_uninitialized_input(_Bool allows_uninitialized_input) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    deprecated(int version, const char* explanation) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_shape_inference_function(
        void (*)(TF_ShapeInferenceContext*, TF_Status*) shape_inference_func
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> register_op_definition() noexcept = 0;

    static TF_OpDefinitionBuilderOps* get_generic_vtable()
    {
        static TF_OpDefinitionBuilderOps vtable = {
            .struct_size = TF_OPDEFINITIONBUILDER_STRUCT_SIZE,
            .add_attr =
                [](TF_OpDefinitionBuilder* builder, const char* attr_spec) noexcept
            {
                auto* self = TF_OpDefinitionBuilderOps::create(builder);
                auto res = self->add_attr(attr_spec);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .add_input =
                [](TF_OpDefinitionBuilder* builder, const char* input_spec) noexcept
            {
                auto* self = TF_OpDefinitionBuilderOps::create(builder);
                auto res = self->add_input(input_spec);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .add_output =
                [](TF_OpDefinitionBuilder* builder, const char* output_spec) noexcept
            {
                auto* self = TF_OpDefinitionBuilderOps::create(builder);
                auto res = self->add_output(output_spec);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_is_commutative =
                [](TF_OpDefinitionBuilder* builder, _Bool is_commutative) noexcept
            {
                auto* self = TF_OpDefinitionBuilderOps::create(builder);
                auto res = self->set_is_commutative(is_commutative);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_is_aggregate =
                [](TF_OpDefinitionBuilder* builder, _Bool is_aggregate) noexcept
            {
                auto* self = TF_OpDefinitionBuilderOps::create(builder);
                auto res = self->set_is_aggregate(is_aggregate);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_is_stateful =
                [](TF_OpDefinitionBuilder* builder, _Bool is_stateful) noexcept
            {
                auto* self = TF_OpDefinitionBuilderOps::create(builder);
                auto res = self->set_is_stateful(is_stateful);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_allows_uninitialized_input =
                [](TF_OpDefinitionBuilder* builder, _Bool allows_uninitialized_input) noexcept
            {
                auto* self = TF_OpDefinitionBuilderOps::create(builder);
                auto res = self->set_allows_uninitialized_input(allows_uninitialized_input);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .deprecated =
                [](TF_OpDefinitionBuilder* builder, int version, const char* explanation) noexcept
            {
                auto* self = TF_OpDefinitionBuilderOps::create(builder);
                auto res = self->deprecated(version, explanation);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_shape_inference_function =
                [](TF_OpDefinitionBuilder* builder,
                   void (*)(TF_ShapeInferenceContext*, TF_Status*) shape_inference_func) noexcept
            {
                auto* self = TF_OpDefinitionBuilderOps::create(builder);
                auto res = self->set_shape_inference_function(shape_inference_func);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .register_op_definition =
                [](TF_OpDefinitionBuilder* builder, TF_Status* out_status) noexcept
            {
                auto* self = TF_OpDefinitionBuilderOps::create(builder);
                auto res = self->register_op_definition();
                if (!res) {
                    res.error().to_c(out_status);
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
