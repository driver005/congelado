// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/shape_inference_context.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/shape_inference_context.h"

export module cc_ice_extern_ops_sonic:shape_inference_context;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ShapeInferenceContextOps :
    public ice::sonic::Runtime<TF_ShapeInferenceContextOps, TF_ShapeInferenceContextOps>
{
public:
    explicit TF_ShapeInferenceContextOps(
        TF_ShapeInferenceContextOps* ops,
        void* plugin_context
    ) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "ops";

    [[nodiscard]] std::expected<void, ice::Status> num_inputs(int64_t* out_num) noexcept
    {
        ice::Status status;
        m_ops->num_inputs(get_handle(), out_num, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_input(int i, TF_ShapeHandle* handle) noexcept
    {
        ice::Status status;
        m_ops->get_input(get_handle(), i, handle, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_output(int i, TF_ShapeHandle* handle) noexcept
    {
        ice::Status status;
        m_ops->set_output(get_handle(), i, handle, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> scalar(TF_ShapeHandle* handle) noexcept
    {
        ice::Status status;
        m_ops->scalar(get_handle(), handle, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    vector_from_size(size_t size, TF_ShapeHandle* handle) noexcept
    {
        ice::Status status;
        m_ops->vector_from_size(get_handle(), size, handle, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_attr_type(const ice::sonic::String& attr_name, TFDataTypeEnum* out_val) noexcept
    {
        ice::Status status;
        m_ops->get_attr_type(get_handle(), attr_name.get_handle(), out_val, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    rank(TF_ShapeHandle* handle, int64_t* out_rank) noexcept
    {
        ice::Status status;
        m_ops->rank(get_handle(), handle, out_rank, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    rank_known(TF_ShapeHandle* handle, int* out_known) noexcept
    {
        ice::Status status;
        m_ops->rank_known(get_handle(), handle, out_known, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    with_rank(TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result) noexcept
    {
        ice::Status status;
        m_ops->with_rank(get_handle(), handle, rank, result, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    with_rank_at_least(TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result) noexcept
    {
        ice::Status status;
        m_ops->with_rank_at_least(get_handle(), handle, rank, result, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    with_rank_at_most(TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result) noexcept
    {
        ice::Status status;
        m_ops->with_rank_at_most(get_handle(), handle, rank, result, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    dim(TF_ShapeHandle* shape_handle,
        int64_t i,
        const ice::sonic::TF_DimensionHandleOps& result) noexcept
    {
        ice::Status status;
        m_ops->dim(get_handle(), shape_handle, i, result.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> subshape(
        TF_ShapeHandle* shape_handle,
        int64_t start,
        int64_t end,
        TF_ShapeHandle* result
    ) noexcept
    {
        ice::Status status;
        m_ops->subshape(get_handle(), shape_handle, start, end, result, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_unknown_shape() noexcept
    {
        ice::Status status;
        m_ops->set_unknown_shape(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> concatenate_shapes(
        TF_ShapeHandle* first,
        TF_ShapeHandle* second,
        TF_ShapeHandle* result
    ) noexcept
    {
        ice::Status status;
        m_ops->concatenate_shapes(get_handle(), first, second, result, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    sonic::String get_name() const noexcept
    {
        sonic::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
