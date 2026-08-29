module;

#include "c/extern/ops.h"

export module cc_abi_builder:shape_inference_context_view;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_builder_intern;
import :shape_handle;
import :dimension_handle;

export namespace ice::builder {

// Borrowed wrapper for TF_ShapeInferenceContext
class ShapeInferenceContextView
{
public:
    explicit ShapeInferenceContextView(TF_ShapeInferenceContext* handle) :
        m_handle{handle}
    {
    }

    ~ShapeInferenceContextView() = default;

    ShapeInferenceContextView(const ShapeInferenceContextView&) = delete;
    ShapeInferenceContextView& operator=(const ShapeInferenceContextView&) = delete;

    ShapeInferenceContextView(ShapeInferenceContextView&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    ShapeInferenceContextView& operator=(ShapeInferenceContextView&& other) noexcept
    {

        if (this != &other) {
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    int64_t num_inputs() const
    {
        return TF_ShapeInferenceContextNumInputs(m_handle);
    }

    [[nodiscard]] std::expected<ShapeHandle, ice::Status> get_input(int i)
    {

        ice::Status status;
        ShapeHandle handle = ShapeHandle::create();
        TF_ShapeInferenceContextGetInput(m_handle, i, handle.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return handle;
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_output(int i, const ShapeHandle& handle)
    {

        ice::Status status;
        TF_ShapeInferenceContextSetOutput(
            m_handle, i, const_cast<TF_ShapeHandle*>(handle.get_handle()), status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ShapeHandle scalar()
    {
        return ShapeHandle(TF_ShapeInferenceContextScalar(m_handle));
    }

    ShapeHandle vector_from_size(size_t size)
    {
        return ShapeHandle(TF_ShapeInferenceContextVectorFromSize(m_handle, size));
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_attr_type(const ice::String& attr_name, DataTypeEnum* val)
    {

        ice::Status status;
        TF_DataType_Enum raw = static_cast<TF_DataType_Enum>(0);
        TF_ShapeInferenceContext_GetAttrType(
            m_handle, attr_name.c_str(), &raw, status.get_handle()
        );
        *val = data_type_from_c(raw);
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    int64_t rank(const ShapeHandle& handle) const
    {
        return TF_ShapeInferenceContextRank(
            m_handle, const_cast<TF_ShapeHandle*>(handle.get_handle())
        );
    }

    int rank_known(const ShapeHandle& handle) const
    {
        return TF_ShapeInferenceContextRankKnown(
            m_handle, const_cast<TF_ShapeHandle*>(handle.get_handle())
        );
    }

    [[nodiscard]] std::expected<void, ice::Status>
    with_rank(const ShapeHandle& handle, int64_t rank, ShapeHandle* result)
    {

        ice::Status status;
        TF_ShapeInferenceContextWithRank(
            m_handle, const_cast<TF_ShapeHandle*>(handle.get_handle()), rank, result->get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    with_rank_at_least(const ShapeHandle& handle, int64_t rank, ShapeHandle* result)
    {

        ice::Status status;
        TF_ShapeInferenceContextWithRankAtLeast(
            m_handle, const_cast<TF_ShapeHandle*>(handle.get_handle()), rank, result->get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    with_rank_at_most(const ShapeHandle& handle, int64_t rank, ShapeHandle* result)
    {

        ice::Status status;
        TF_ShapeInferenceContextWithRankAtMost(
            m_handle, const_cast<TF_ShapeHandle*>(handle.get_handle()), rank, result->get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void dim(const ShapeHandle& shape_handle, int64_t i, DimensionHandle* result)
    {

        TF_ShapeInferenceContextDim(
            m_handle, const_cast<TF_ShapeHandle*>(shape_handle.get_handle()), i,
            result->get_handle()
        );
    }

    [[nodiscard]] std::expected<void, ice::Status> subshape(
        const ShapeHandle& shape_handle, int64_t start, int64_t end, ShapeHandle* result
    )
    {

        ice::Status status;
        TF_ShapeInferenceContextSubshape(
            m_handle, const_cast<TF_ShapeHandle*>(shape_handle.get_handle()), start, end,
            result->get_handle(), status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_unknown_shape()
    {

        ice::Status status;
        TF_ShapeInferenceContextSetUnknownShape(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    bool dimension_value_known(const DimensionHandle& dim_handle) const
    {
        return TF_DimensionHandleValueKnown(
            const_cast<TF_DimensionHandle*>(dim_handle.get_handle())
        );
    }

    int64_t dimension_value(const DimensionHandle& dim_handle) const
    {
        return TF_DimensionHandleValue(const_cast<TF_DimensionHandle*>(dim_handle.get_handle()));
    }

    [[nodiscard]] std::expected<void, ice::Status> concatenate_shapes(
        const ShapeHandle& first, const ShapeHandle& second, ShapeHandle* result
    )
    {

        ice::Status status;
        TF_ShapeInferenceContextConcatenateShapes(
            m_handle, const_cast<TF_ShapeHandle*>(first.get_handle()),
            const_cast<TF_ShapeHandle*>(second.get_handle()), result->get_handle(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    // Underlying handle — pass directly to the C ABI
    TF_ShapeInferenceContext* get_handle()
    {
        return m_handle;
    }

    const TF_ShapeInferenceContext* get_handle() const
    {
        return m_handle;
    }

private:
    TF_ShapeInferenceContext* m_handle;
};

} // namespace ice::builder
