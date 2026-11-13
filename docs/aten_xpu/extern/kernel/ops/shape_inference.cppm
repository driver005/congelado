module;

#include "include/c/extern/ops/shape_handle.h"
#include "include/c/extern/ops/shape_inference_context.h"
#include "include/c/intern/status.h"

export module aten_xpu_extern_kernel:ops_shape_inference;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_ops_sonic;
import aten_xpu_intern;

export namespace aten_xpu {

// Output shape formulas for the plugin's ops; every output not named by a rule stays unknown.
enum class SyclShapeRule
{
    k_unknown,
    k_matmul,
    k_batch_matmul,
    k_addend_matmul,
    k_addend_batch_matmul,
    k_addend_matrix_vector,
    k_linear,
    k_convolution,
    k_transposed_convolution,
    k_quantized_matmul,
    k_weight_only_matmul,
    k_attention,
    k_attention_gradient,
    k_convolution_gradient,
    k_recurrent
};

// Runs one shape rule inside the host's shape inference callback.
class SyclShapeInference
{
public:
    template<SyclShapeRule Rule>
    static void infer(::TF_ShapeInferenceContext* context, ::TF_Status* status)
    {
        SyclShapeInference inference{context, status};
        inference.apply(Rule);
    }

    ~SyclShapeInference()
    {
        for (const auto& shape: m_shapes) {
            shape.destroy();
        }
    }

    SyclShapeInference(const SyclShapeInference&) = delete;
    SyclShapeInference& operator=(const SyclShapeInference&) = delete;
    SyclShapeInference(SyclShapeInference&&) = delete;
    SyclShapeInference& operator=(SyclShapeInference&&) = delete;

private:
    SyclShapeInference(::TF_ShapeInferenceContext* context, ::TF_Status* status) :
        m_ops{SyclOpsTable::getInstance()},
        m_context{m_ops.getShapeInferenceContextOps(), context},
        m_status{m_ops.getStatusOps(), status}
    {
    }

    void apply(SyclShapeRule rule)
    {
        m_context.set_unknown_shape(m_status);
        switch (rule) {
            case SyclShapeRule::k_unknown:
                return;
            case SyclShapeRule::k_matmul:
                matmul(0);
                return;
            case SyclShapeRule::k_batch_matmul:
                batchMatmul(0);
                return;
            case SyclShapeRule::k_addend_matmul:
                matmul(1);
                return;
            case SyclShapeRule::k_addend_batch_matmul:
                batchMatmul(1);
                return;
            case SyclShapeRule::k_addend_matrix_vector:
                matrixVector(1);
                return;
            case SyclShapeRule::k_linear:
                linear();
                return;
            case SyclShapeRule::k_convolution:
                convolution();
                return;
            case SyclShapeRule::k_transposed_convolution:
                transposedConvolution();
                return;
            case SyclShapeRule::k_quantized_matmul:
                quantizedMatmul();
                return;
            case SyclShapeRule::k_weight_only_matmul:
                weightOnlyMatmul();
                return;
            case SyclShapeRule::k_attention:
                attention();
                return;
            case SyclShapeRule::k_attention_gradient:
                passThrough(1, 0);
                passThrough(2, 1);
                passThrough(3, 2);
                return;
            case SyclShapeRule::k_convolution_gradient:
                convolutionGradient();
                return;
            case SyclShapeRule::k_recurrent:
                passThrough(1, 1);
                passThrough(2, 2);
                return;
        }
    }

    // [rows, inner] x [inner, columns] -> [rows, columns]
    void matmul(int first)
    {
        const auto& left = ranked(first, 2);
        const auto& right = ranked(first + 1, 2);
        setOutput(0, join(slice(left, 0, 1), slice(right, 1, 2)));
    }

    // [batch, rows, inner] x [batch, inner, columns] -> [batch, rows, columns]
    void batchMatmul(int first)
    {
        const auto& left = ranked(first, 3);
        const auto& right = ranked(first + 1, 3);
        setOutput(0, join(slice(left, 0, 2), slice(right, 2, 3)));
    }

    // [rows, columns] x [columns] -> [rows]
    void matrixVector(int matrix)
    {
        setOutput(0, slice(ranked(matrix, 2), 0, 1));
    }

    // [..., in_features] x [out_features, in_features] -> [..., out_features]
    void linear()
    {
        const auto& source = input(0);
        const auto& weight = ranked(1, 2);
        setOutput(0, join(slice(source, 0, -1), slice(weight, 0, 1)));
    }

    // [batch, out_channels, ...] with unknown spatial extents: the shape context cannot read the
    // stride, padding and dilation attrs
    void convolution()
    {
        const auto& source = input(0);
        const std::int64_t rank = rankOf(source);
        if (!ok() || rank < 2) {
            return;
        }
        const auto& weight = ranked(1, rank);
        const auto& leading = join(slice(source, 0, 1), slice(weight, 0, 1));
        setOutput(0, join(leading, unknownDims(rank - 2)));
    }

    // [batch, ...] only: the output channels are weight[1] * groups and groups is an attr
    void transposedConvolution()
    {
        const auto& source = input(0);
        const std::int64_t rank = rankOf(source);
        if (!ok() || rank < 2) {
            return;
        }
        setOutput(0, join(slice(source, 0, 1), unknownDims(rank - 1)));
    }

    // [..., unknown]: which weight axis holds the columns depends on the weight_transposed attr
    void quantizedMatmul()
    {
        const auto& source = input(0);
        if (rankOf(source) < 1) {
            return;
        }
        setOutput(0, join(slice(source, 0, -1), unknownDims(1)));
    }

    // [..., columns] where columns is the last extent of the scale input
    void weightOnlyMatmul()
    {
        const auto& source = input(0);
        const auto& scale = input(2);
        const std::int64_t scale_rank = rankOf(scale);
        if (!ok() || rankOf(source) < 1 || scale_rank < 1) {
            return;
        }
        setOutput(0, join(slice(source, 0, -1), slice(scale, scale_rank - 1, scale_rank)));
    }

    // [batch, heads, length, head_dim] query with the value head_dim
    void attention()
    {
        const auto& query = ranked(0, 4);
        const auto& value = ranked(2, 4);
        setOutput(0, join(slice(query, 0, 3), slice(value, 3, 4)));
    }

    // gradients take the shape of source and weight; the bias gradient is [out_channels]
    void convolutionGradient()
    {
        passThrough(1, 0);
        passThrough(2, 1);
        setOutput(2, slice(input(2), 0, 1));
    }

    void passThrough(int source, int target)
    {
        setOutput(target, input(source));
    }

    const ice::sonic::ShapeHandle& input(int index)
    {
        const auto& shape = makeShape();
        if (ok()) {
            m_context.get_input(index, shape, m_status);
        }
        return shape;
    }

    std::int64_t rankOf(const ice::sonic::ShapeHandle& shape)
    {
        std::int64_t rank = -1;
        if (ok()) {
            m_context.rank(shape, rank);
        }
        return rank;
    }

    const ice::sonic::ShapeHandle& unknownDims(std::int64_t count)
    {
        const auto& shape = makeShape();
        if (ok()) {
            m_context.vector_from_size(static_cast<std::size_t>(count), shape);
        }
        return shape;
    }

    const ice::sonic::ShapeHandle& ranked(int index, std::int64_t rank)
    {
        const auto& source = input(index);
        const auto& shape = makeShape();
        if (ok()) {
            m_context.with_rank(source, rank, shape, m_status);
        }
        return shape;
    }

    const ice::sonic::ShapeHandle&
    slice(const ice::sonic::ShapeHandle& source, std::int64_t start, std::int64_t end)
    {
        const auto& shape = makeShape();
        if (ok()) {
            m_context.subshape(source, start, end, shape, m_status);
        }
        return shape;
    }

    const ice::sonic::ShapeHandle&
    join(const ice::sonic::ShapeHandle& first, const ice::sonic::ShapeHandle& second)
    {
        const auto& shape = makeShape();
        if (ok()) {
            m_context.concatenate_shapes(first, second, shape, m_status);
        }
        return shape;
    }

    void setOutput(int index, const ice::sonic::ShapeHandle& shape)
    {
        if (ok()) {
            m_context.set_output(index, shape, m_status);
        }
    }

    const ice::sonic::ShapeHandle& makeShape()
    {
        const auto& shape = m_shapes.emplace_back(m_ops.getShapeHandleOps());
        shape.create();
        return shape;
    }

    bool ok() const noexcept
    {
        ice::sonic::Code code{ice::sonic::Code::k_ok};
        m_status.get_code(code);
        return code.get_value() == TF_OK;
    }

    const SyclOpsTable& m_ops;
    ice::sonic::ShapeInferenceContext m_context;
    ice::sonic::Status m_status;
    std::deque<ice::sonic::ShapeHandle> m_shapes;
};

} // namespace aten_xpu
