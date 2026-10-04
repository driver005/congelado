module;

#include "include/c/intern/datatype.h"

export module aten_xpu_extern_kernel:convolution_conv;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;
import :onednn_post_op_attributes;
import :onednn_fusion;
import :onednn_convolution_geometry;
import :onednn_convolution_primitive;

export namespace aten_xpu {

class SyclConvolutionKernel : public SyclKernel<SyclConvolutionKernel>
{
public:
    static constexpr std::string_view k_name = "Conv2D";

    SyclConvolutionKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclConvolutionKernel>{ops},
        m_geometry{SyclConvolutionGeometry::from_construction(construction)},
        m_unary{construction.getString("unary", "none")},
        m_algorithm{construction.getString("algorithm", "none")},
        m_binary{construction.getString("binary", "none")}
    {

        m_scalar_count = construction.getFloatList("scalars", m_scalars);
        if (m_unary == "none") {
            m_unary = SyclFusion::activation_name(construction.getInt64("activation", 0));
        }

    }

    void compute(SyclKernelContext& context)
    {

        auto source = context.getInput(0);
        auto weight = context.getInput(1);
        auto queue = context.getQueue();
        if (!source || !weight || !queue) {
            context.propagate();
            return;
        }
        auto bias = context.getInputCount() > 2 ? context.getInput(2) : std::nullopt;
        auto other = context.getInputCount() > 3 ? context.getInput(3) : std::nullopt;

        m_geometry.setSpatialRank(source->get().getDims().size() - 2);
        const auto dims = m_geometry.output_dims(source->get().getDims(), weight->get().getDims());
        m_scratch_dims.assign(dims.begin(), dims.end());
        auto output = context.allocateOutput(0, source->get().getDtype(), m_scratch_dims);
        if (!output) {
            context.propagate();
            return;
        }

        SyclPostOpAttributes attributes;
        if (!build_post_ops(attributes, other)) {
            context.fail(TF_INVALID_ARGUMENT, "unsupported convolution post op");
            return;
        }
        SyclConvolutionPrimitive::forward(
            queue->get(),
            source->get(),
            weight->get(),
            bias ? std::optional<std::reference_wrapper<const SyclTensor>>{bias->get()} : std::nullopt,
            output->get(),
            m_geometry,
            attributes
        );

    }

private:
    bool build_post_ops(
        SyclPostOpAttributes& attributes,
        std::optional<std::reference_wrapper<SyclTensor>> other
    ) const
    {

        const auto scalars = std::span<const float>{m_scalars}.first(m_scalar_count);
        if (m_binary == "none") {
            return SyclFusion::add_unary(attributes, m_unary, scalars, m_algorithm);
        }
        return SyclFusion::add_binary_then_unary(
            attributes,
            m_binary == "add" ? "add" : "sum",
            1.0F,
            0,
            other ? std::optional<std::reference_wrapper<const SyclTensor>>{other->get()} : std::nullopt,
            m_unary,
            scalars,
            m_algorithm
        );

    }

    SyclConvolutionGeometry m_geometry;
    std::string m_unary;
    std::string m_algorithm;
    std::string m_binary;
    std::array<float, 4> m_scalars{};
    std::size_t m_scalar_count{0};
    std::vector<int64_t> m_scratch_dims;
};

} // namespace aten_xpu
