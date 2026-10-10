// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/shape_inference_context.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/dimension_handle.h"
#include "include/c/extern/ops/shape_handle.h"
#include "include/c/extern/ops/shape_inference_context.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_ops_builder:shape_inference_context;

import std;
import cc_ice_extern_ops_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ShapeInferenceContextOps
{
public:
    explicit TF_ShapeInferenceContextOps(
        const ::TF_DimensionHandleOps* TF_DimensionHandleOps_ops,
        const ::TF_ShapeHandleOps* TF_ShapeHandleOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_DimensionHandleOps_ops = TF_DimensionHandleOps_ops;
        m_TF_ShapeHandleOps_ops = TF_ShapeHandleOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TF_ShapeInferenceContextOps(const TF_ShapeInferenceContextOps&) = delete;
    TF_ShapeInferenceContextOps& operator=(const TF_ShapeInferenceContextOps&) = delete;

    static TF_ShapeInferenceContextOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ShapeInferenceContextOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ShapeInferenceContextOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ShapeInferenceContextOps*>(handle->plugin_data);
    }

    virtual ~TF_ShapeInferenceContextOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void num_inputs(int64_t* out_num) noexcept = 0;
    virtual void get_input(
        int i,
        const ice::sonic::TF_ShapeHandleOps& handle,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_output(
        int i,
        const ice::sonic::TF_ShapeHandleOps& handle,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void scalar(const ice::sonic::TF_ShapeHandleOps& handle) noexcept = 0;
    virtual void
    vector_from_size(size_t size, const ice::sonic::TF_ShapeHandleOps& handle) noexcept = 0;
    virtual void get_attr_type(
        const ice::sonic::String& attr_name,
        TFDataTypeEnum* out_val,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void rank(const ice::sonic::TF_ShapeHandleOps& handle, int64_t* out_rank) noexcept = 0;
    virtual void
    rank_known(const ice::sonic::TF_ShapeHandleOps& handle, int* out_known) noexcept = 0;
    virtual void with_rank(
        const ice::sonic::TF_ShapeHandleOps& handle,
        int64_t rank,
        const ice::sonic::TF_ShapeHandleOps& result,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void with_rank_at_least(
        const ice::sonic::TF_ShapeHandleOps& handle,
        int64_t rank,
        const ice::sonic::TF_ShapeHandleOps& result,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void with_rank_at_most(
        const ice::sonic::TF_ShapeHandleOps& handle,
        int64_t rank,
        const ice::sonic::TF_ShapeHandleOps& result,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    dim(const ice::sonic::TF_ShapeHandleOps& shape_handle,
        int64_t i,
        const ice::sonic::TF_DimensionHandleOps& result) noexcept = 0;
    virtual void subshape(
        const ice::sonic::TF_ShapeHandleOps& shape_handle,
        int64_t start,
        int64_t end,
        const ice::sonic::TF_ShapeHandleOps& result,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_unknown_shape(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void concatenate_shapes(
        const ice::sonic::TF_ShapeHandleOps& first,
        const ice::sonic::TF_ShapeHandleOps& second,
        const ice::sonic::TF_ShapeHandleOps& result,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_ShapeInferenceContext*)) noexcept
    {
        m_vtable = ::TF_ShapeInferenceContextOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ShapeInferenceContextOps, concatenate_shapes),

            .create = create,
            .destroy =
                [](TF_ShapeInferenceContext* handle) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(handle);
                self.destroy();
            },
            .num_inputs =
                [](TF_ShapeInferenceContext* ctx, int64_t* out_num) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.num_inputs(out_num);
            },
            .get_input =
                [](TF_ShapeInferenceContext* ctx,
                   int i,
                   TF_ShapeHandle* handle,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.get_input(
                    i,
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, handle),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_output =
                [](TF_ShapeInferenceContext* ctx,
                   int i,
                   TF_ShapeHandle* handle,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.set_output(
                    i,
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, handle),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .scalar =
                [](TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.scalar(self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, handle));
            },
            .vector_from_size =
                [](TF_ShapeInferenceContext* ctx, size_t size, TF_ShapeHandle* handle) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.vector_from_size(
                    size,
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, handle)
                );
            },
            .get_attr_type =
                [](TF_ShapeInferenceContext* ctx,
                   const TF_String* attr_name,
                   TFDataTypeEnum* out_val,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.get_attr_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_val,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .rank =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* handle,
                   int64_t* out_rank) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.rank(
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, handle),
                    out_rank
                );
            },
            .rank_known =
                [](TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle, int* out_known) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.rank_known(
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, handle),
                    out_known
                );
            },
            .with_rank =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* handle,
                   int64_t rank,
                   TF_ShapeHandle* result,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.with_rank(
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, handle),
                    rank,
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, result),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .with_rank_at_least =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* handle,
                   int64_t rank,
                   TF_ShapeHandle* result,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.with_rank_at_least(
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, handle),
                    rank,
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, result),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .with_rank_at_most =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* handle,
                   int64_t rank,
                   TF_ShapeHandle* result,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.with_rank_at_most(
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, handle),
                    rank,
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, result),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .dim =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* shape_handle,
                   int64_t i,
                   TF_DimensionHandle* result) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.dim(
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, shape_handle),
                    i,
                    self.wrap(std::type_identity<ice::sonic::TF_DimensionHandleOps>{}, result)
                );
            },
            .subshape =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* shape_handle,
                   int64_t start,
                   int64_t end,
                   TF_ShapeHandle* result,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.subshape(
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, shape_handle),
                    start,
                    end,
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, result),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_unknown_shape =
                [](TF_ShapeInferenceContext* ctx, TF_Status* out_status) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.set_unknown_shape(
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .concatenate_shapes =
                [](TF_ShapeInferenceContext* ctx,
                   TF_ShapeHandle* first,
                   TF_ShapeHandle* second,
                   TF_ShapeHandle* result,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ShapeInferenceContextOps::from_handle(ctx);
                self.concatenate_shapes(
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, first),
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, second),
                    self.wrap(std::type_identity<ice::sonic::TF_ShapeHandleOps>{}, result),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_DimensionHandleOps wrap(
        std::type_identity<ice::sonic::TF_DimensionHandleOps>,
        const ::TF_DimensionHandle* handle
    ) const noexcept
    {
        return ice::sonic::TF_DimensionHandleOps{
            m_TF_DimensionHandleOps_ops,
            const_cast<::TF_DimensionHandle*>(handle)
        };
    }

    ice::sonic::TF_ShapeHandleOps wrap(
        std::type_identity<ice::sonic::TF_ShapeHandleOps>,
        const ::TF_ShapeHandle* handle
    ) const noexcept
    {
        return ice::sonic::TF_ShapeHandleOps{
            m_TF_ShapeHandleOps_ops,
            const_cast<::TF_ShapeHandle*>(handle)
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

    const ::TF_ShapeInferenceContextOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_ShapeInferenceContext& get_handle() const noexcept
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
            const_cast<::TF_ShapeInferenceContextOps*>(&m_vtable)
        );
    }

private:
    ::TF_ShapeInferenceContextOps m_vtable;
    ::TF_ShapeInferenceContext m_handle;

    const ::TF_DimensionHandleOps* m_TF_DimensionHandleOps_ops{nullptr};

    const ::TF_ShapeHandleOps* m_TF_ShapeHandleOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
