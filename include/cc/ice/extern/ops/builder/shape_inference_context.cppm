// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/shape_inference_context.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/dimension_handle.h"
#include "include/c/extern/ops/shape_handle.h"
#include "include/c/extern/ops/shape_inference_context.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_ops_builder:shape_inference_context;

import std;
import cc_ice_extern_ops_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class ShapeInferenceContext
{
public:
    explicit ShapeInferenceContext(
        const ::TF_DimensionHandleOps* DimensionHandle_ops,
        const ::TF_ShapeHandleOps* ShapeHandle_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_DimensionHandle_ops = DimensionHandle_ops;
        m_ShapeHandle_ops = ShapeHandle_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    ShapeInferenceContext(const ShapeInferenceContext&) = delete;
    ShapeInferenceContext& operator=(const ShapeInferenceContext&) = delete;

    static ShapeInferenceContext& from_handle(void* ctx) noexcept
    {
        return *static_cast<ShapeInferenceContext*>(ctx);
    }

    template<typename HandleT>
    static ShapeInferenceContext& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<ShapeInferenceContext*>(handle->plugin_data);
    }

    virtual ~ShapeInferenceContext() = default;
    virtual void destroy() noexcept = 0;
    virtual void num_inputs(std::int64_t& out_num) noexcept = 0;
    virtual void get_input(
        int i,
        const ice::sonic::ShapeHandle& handle,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_output(
        int i,
        const ice::sonic::ShapeHandle& handle,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void scalar(const ice::sonic::ShapeHandle& handle) noexcept = 0;
    virtual void vector_from_size(size_t size, const ice::sonic::ShapeHandle& handle) noexcept = 0;
    virtual void get_attr_type(
        const ice::sonic::String& attr_name,
        ::TFDataTypeEnum& out_val,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void rank(const ice::sonic::ShapeHandle& handle, std::int64_t& out_rank) noexcept = 0;
    virtual void rank_known(const ice::sonic::ShapeHandle& handle, int& out_known) noexcept = 0;
    virtual void with_rank(
        const ice::sonic::ShapeHandle& handle,
        int64_t rank,
        const ice::sonic::ShapeHandle& result,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void with_rank_at_least(
        const ice::sonic::ShapeHandle& handle,
        int64_t rank,
        const ice::sonic::ShapeHandle& result,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void with_rank_at_most(
        const ice::sonic::ShapeHandle& handle,
        int64_t rank,
        const ice::sonic::ShapeHandle& result,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    dim(const ice::sonic::ShapeHandle& shape_handle,
        int64_t i,
        const ice::sonic::DimensionHandle& result) noexcept = 0;
    virtual void subshape(
        const ice::sonic::ShapeHandle& shape_handle,
        int64_t start,
        int64_t end,
        const ice::sonic::ShapeHandle& result,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_unknown_shape(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void concatenate_shapes(
        const ice::sonic::ShapeHandle& first,
        const ice::sonic::ShapeHandle& second,
        const ice::sonic::ShapeHandle& result,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_ShapeInferenceContext*)) noexcept
    {
        m_vtable = ::TF_ShapeInferenceContextOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ShapeInferenceContextOps, concatenate_shapes),

            .create = create,
            .destroy =
                [](::TF_ShapeInferenceContext* handle) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(handle);
                self.destroy();
            },
            .num_inputs =
                [](::TF_ShapeInferenceContext* ctx, int64_t* out_num) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.num_inputs(*out_num);
            },
            .get_input =
                [](::TF_ShapeInferenceContext* ctx,
                   int i,
                   ::TF_ShapeHandle* handle,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.get_input(
                    i,
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, handle),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_output =
                [](::TF_ShapeInferenceContext* ctx,
                   int i,
                   ::TF_ShapeHandle* handle,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.set_output(
                    i,
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, handle),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .scalar =
                [](::TF_ShapeInferenceContext* ctx, ::TF_ShapeHandle* handle) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.scalar(self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, handle));
            },
            .vector_from_size =
                [](::TF_ShapeInferenceContext* ctx, size_t size, ::TF_ShapeHandle* handle) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.vector_from_size(
                    size,
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, handle)
                );
            },
            .get_attr_type =
                [](::TF_ShapeInferenceContext* ctx,
                   const ::TF_String* attr_name,
                   ::TFDataTypeEnum* out_val,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.get_attr_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    *out_val,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .rank =
                [](::TF_ShapeInferenceContext* ctx,
                   ::TF_ShapeHandle* handle,
                   int64_t* out_rank) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.rank(
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, handle),
                    *out_rank
                );
            },
            .rank_known =
                [](::TF_ShapeInferenceContext* ctx,
                   ::TF_ShapeHandle* handle,
                   int* out_known) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.rank_known(
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, handle),
                    *out_known
                );
            },
            .with_rank =
                [](::TF_ShapeInferenceContext* ctx,
                   ::TF_ShapeHandle* handle,
                   int64_t rank,
                   ::TF_ShapeHandle* result,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.with_rank(
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, handle),
                    rank,
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, result),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .with_rank_at_least =
                [](::TF_ShapeInferenceContext* ctx,
                   ::TF_ShapeHandle* handle,
                   int64_t rank,
                   ::TF_ShapeHandle* result,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.with_rank_at_least(
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, handle),
                    rank,
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, result),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .with_rank_at_most =
                [](::TF_ShapeInferenceContext* ctx,
                   ::TF_ShapeHandle* handle,
                   int64_t rank,
                   ::TF_ShapeHandle* result,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.with_rank_at_most(
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, handle),
                    rank,
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, result),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .dim =
                [](::TF_ShapeInferenceContext* ctx,
                   ::TF_ShapeHandle* shape_handle,
                   int64_t i,
                   ::TF_DimensionHandle* result) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.dim(
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, shape_handle),
                    i,
                    self.wrap(std::type_identity<ice::sonic::DimensionHandle>{}, result)
                );
            },
            .subshape =
                [](::TF_ShapeInferenceContext* ctx,
                   ::TF_ShapeHandle* shape_handle,
                   int64_t start,
                   int64_t end,
                   ::TF_ShapeHandle* result,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.subshape(
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, shape_handle),
                    start,
                    end,
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, result),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_unknown_shape =
                [](::TF_ShapeInferenceContext* ctx, ::TF_Status* out_status) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.set_unknown_shape(
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .concatenate_shapes =
                [](::TF_ShapeInferenceContext* ctx,
                   ::TF_ShapeHandle* first,
                   ::TF_ShapeHandle* second,
                   ::TF_ShapeHandle* result,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = ShapeInferenceContext::from_handle(ctx);
                self.concatenate_shapes(
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, first),
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, second),
                    self.wrap(std::type_identity<ice::sonic::ShapeHandle>{}, result),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::DimensionHandle wrap(
        std::type_identity<ice::sonic::DimensionHandle>,
        const ::TF_DimensionHandle* handle
    ) const noexcept
    {
        return ice::sonic::DimensionHandle{
            m_DimensionHandle_ops,
            const_cast<::TF_DimensionHandle*>(handle)
        };
    }

    ice::sonic::ShapeHandle
    wrap(std::type_identity<ice::sonic::ShapeHandle>, const ::TF_ShapeHandle* handle) const noexcept
    {
        return ice::sonic::ShapeHandle{m_ShapeHandle_ops, const_cast<::TF_ShapeHandle*>(handle)};
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
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_ShapeInferenceContextOps*>(&m_vtable));
    }

private:
    ::TF_ShapeInferenceContextOps m_vtable;
    ::TF_ShapeInferenceContext m_handle;

    const ::TF_DimensionHandleOps* m_DimensionHandle_ops{nullptr};

    const ::TF_ShapeHandleOps* m_ShapeHandle_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
