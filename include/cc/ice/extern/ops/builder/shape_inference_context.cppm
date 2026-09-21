// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/shape_inference_context.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/shape_inference_context.h"

export module cc_ice_extern_ops_builder:shape_inference_context;

import std;

export namespace ice::builder {

class TF_ShapeInferenceContextOps
{
public:
    static TF_ShapeInferenceContextOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ShapeInferenceContextOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ShapeInferenceContextOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ShapeInferenceContextOps*>(handle->plugin_data);
    }

    virtual ~TF_ShapeInferenceContextOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    num_inputs(int64_t* out_num) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_input(int i, TF_ShapeHandle* handle) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_output(int i, TF_ShapeHandle* handle) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    scalar(TF_ShapeHandle* handle) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    vector_from_size(size_t size, TF_ShapeHandle* handle) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_type(const char* attr_name, TFDataTypeEnum* out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    rank(TF_ShapeHandle* handle, int64_t* out_rank) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    rank_known(TF_ShapeHandle* handle, int* out_known) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    with_rank(TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    with_rank_at_least(TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    with_rank_at_most(TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    dim(TF_ShapeHandle* shape_handle,
        int64_t i,
        const ice::sonic::TF_DimensionHandleOps& result) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> subshape(
        TF_ShapeHandle* shape_handle,
        int64_t start,
        int64_t end,
        TF_ShapeHandle* result
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_unknown_shape() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> concatenate_shapes(
        TF_ShapeHandle* first,
        TF_ShapeHandle* second,
        TF_ShapeHandle* result
    ) noexcept = 0;

    static TF_ShapeInferenceContextOps* get_generic_vtable()
    {
        static TF_ShapeInferenceContextOps vtable = {
            .struct_size = TF_SHAPEINFERENCECONTEXT_STRUCT_SIZE,
            .num_inputs =
                [](TF_ShapeInferenceContext* ctx, int64_t* out_num) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->num_inputs(out_num);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_input =
                [](TF_ShapeInferenceContext* ctx,
                   int i,
                   TF_ShapeHandle* handle,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->get_input(i, handle);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_output =
                [](TF_ShapeInferenceContext* ctx,
                   int i,
                   TF_ShapeHandle* handle,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->set_output(i, handle);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .scalar =
                [](TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->scalar(handle);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .vector_from_size =
                [](TF_ShapeInferenceContext* ctx, size_t size, TF_ShapeHandle* handle) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->vector_from_size(size, handle);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_attr_type =
                [](TF_ShapeInferenceContext* ctx,
                   const char* attr_name,
                   TFDataTypeEnum* out_val,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->get_attr_type(attr_name, out_val);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .rank =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* handle,
                   int64_t* out_rank) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->rank(handle, out_rank);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .rank_known =
                [](TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle, int* out_known) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->rank_known(handle, out_known);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .with_rank =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* handle,
                   int64_t rank,
                   TF_ShapeHandle* result,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->with_rank(handle, rank, result);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .with_rank_at_least =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* handle,
                   int64_t rank,
                   TF_ShapeHandle* result,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->with_rank_at_least(handle, rank, result);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .with_rank_at_most =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* handle,
                   int64_t rank,
                   TF_ShapeHandle* result,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->with_rank_at_most(handle, rank, result);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .dim =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* shape_handle,
                   int64_t i,
                   TF_DimensionHandle* result) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res =
                    self->dim(shape_handle, i, ice::sonic::TF_DimensionHandleOps::wrap(result));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .subshape =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* shape_handle,
                   int64_t start,
                   int64_t end,
                   TF_ShapeHandle* result,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->subshape(shape_handle, start, end, result);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_unknown_shape =
                [](TF_ShapeInferenceContext* ctx, TF_Status* out_status) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->set_unknown_shape();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .concatenate_shapes =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* first,
                   TF_ShapeHandle* second,
                   TF_ShapeHandle* result,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ShapeInferenceContextOps::create(ctx);
                auto res = self->concatenate_shapes(first, second, result);
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
