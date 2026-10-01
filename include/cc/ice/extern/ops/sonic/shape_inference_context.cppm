// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/shape_inference_context.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/shape_inference_context.h"

export module cc_ice_extern_ops_sonic:shape_inference_context;

import std;
import :dimension_handle;
import :shape_handle;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_ShapeInferenceContextOps :
    public ice::sonic::Runtime<::TF_ShapeInferenceContextOps, ::TF_ShapeInferenceContext>
{
public:
    template<typename Registry>
    TF_ShapeInferenceContextOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_ShapeInferenceContextOps(
        Registry& registry,
        ::TF_ShapeInferenceContext* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_ShapeInferenceContextOps(const ::TF_ShapeInferenceContextOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ShapeInferenceContextOps(
        const ::TF_ShapeInferenceContextOps* ops,
        ::TF_ShapeInferenceContext* handle
    ) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void num_inputs(int64_t* out_num) const noexcept
    {
        m_ops->num_inputs(get_handle(), out_num);
    }

    void get_input(
        int i,
        const ice::sonic::TF_ShapeHandleOps& handle,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_input(get_handle(), i, handle.get_handle(), out_status.get_handle());
    }

    void set_output(
        int i,
        const ice::sonic::TF_ShapeHandleOps& handle,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_output(get_handle(), i, handle.get_handle(), out_status.get_handle());
    }

    void scalar(const ice::sonic::TF_ShapeHandleOps& handle) const noexcept
    {
        m_ops->scalar(get_handle(), handle.get_handle());
    }

    void vector_from_size(size_t size, const ice::sonic::TF_ShapeHandleOps& handle) const noexcept
    {
        m_ops->vector_from_size(get_handle(), size, handle.get_handle());
    }

    void get_attr_type(
        const ice::sonic::String& attr_name,
        TFDataTypeEnum* out_val,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops
            ->get_attr_type(get_handle(), attr_name.get_handle(), out_val, out_status.get_handle());
    }

    void rank(const ice::sonic::TF_ShapeHandleOps& handle, int64_t* out_rank) const noexcept
    {
        m_ops->rank(get_handle(), handle.get_handle(), out_rank);
    }

    void rank_known(const ice::sonic::TF_ShapeHandleOps& handle, int* out_known) const noexcept
    {
        m_ops->rank_known(get_handle(), handle.get_handle(), out_known);
    }

    void with_rank(
        const ice::sonic::TF_ShapeHandleOps& handle,
        int64_t rank,
        const ice::sonic::TF_ShapeHandleOps& result,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->with_rank(
            get_handle(),
            handle.get_handle(),
            rank,
            result.get_handle(),
            out_status.get_handle()
        );
    }

    void with_rank_at_least(
        const ice::sonic::TF_ShapeHandleOps& handle,
        int64_t rank,
        const ice::sonic::TF_ShapeHandleOps& result,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->with_rank_at_least(
            get_handle(),
            handle.get_handle(),
            rank,
            result.get_handle(),
            out_status.get_handle()
        );
    }

    void with_rank_at_most(
        const ice::sonic::TF_ShapeHandleOps& handle,
        int64_t rank,
        const ice::sonic::TF_ShapeHandleOps& result,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->with_rank_at_most(
            get_handle(),
            handle.get_handle(),
            rank,
            result.get_handle(),
            out_status.get_handle()
        );
    }

    void dim(
        const ice::sonic::TF_ShapeHandleOps& shape_handle,
        int64_t i,
        const ice::sonic::TF_DimensionHandleOps& result
    ) const noexcept
    {
        m_ops->dim(get_handle(), shape_handle.get_handle(), i, result.get_handle());
    }

    void subshape(
        const ice::sonic::TF_ShapeHandleOps& shape_handle,
        int64_t start,
        int64_t end,
        const ice::sonic::TF_ShapeHandleOps& result,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->subshape(
            get_handle(),
            shape_handle.get_handle(),
            start,
            end,
            result.get_handle(),
            out_status.get_handle()
        );
    }

    void set_unknown_shape(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_unknown_shape(get_handle(), out_status.get_handle());
    }

    void concatenate_shapes(
        const ice::sonic::TF_ShapeHandleOps& first,
        const ice::sonic::TF_ShapeHandleOps& second,
        const ice::sonic::TF_ShapeHandleOps& result,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->concatenate_shapes(
            get_handle(),
            first.get_handle(),
            second.get_handle(),
            result.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
