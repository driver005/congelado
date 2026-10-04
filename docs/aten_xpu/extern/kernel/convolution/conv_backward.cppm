module;

#include "include/c/intern/datatype.h"

export module aten_xpu_extern_kernel:convolution_conv_backward;

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

class SyclConvolutionBackwardKernel : public SyclKernel<SyclConvolutionBackwardKernel>
{
public:
    static constexpr std::string_view k_name = "ConvolutionBackward";

    SyclConvolutionBackwardKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclConvolutionBackwardKernel>{ops},
        m_geometry{SyclConvolutionGeometry::from_construction(construction)},
        m_input_gradient{construction.getBool("input_gradient", true)},
        m_weight_gradient{construction.getBool("weight_gradient", true)},
        m_bias_gradient{construction.getBool("bias_gradient", true)}
    {
    }

    void compute(SyclKernelContext& context)
    {

        auto gradient_output = context.getInput(0);
        auto source = context.getInput(1);
        auto weight = context.getInput(2);
        auto queue = context.getQueue();
        if (!gradient_output || !source || !weight || !queue) {
            context.propagate();
            return;
        }
        m_geometry.setSpatialRank(source->get().getDims().size() - 2);

        if (m_input_gradient) {
            auto gradient_input = context.allocateOutput(0, source->get().getDtype(), source->get().getDims());
            if (!gradient_input) {
                context.propagate();
                return;
            }
            SyclConvolutionPrimitive::backward_data(
                queue->get(),
                gradient_output->get(),
                weight->get(),
                gradient_input->get(),
                m_geometry
            );
        }

        if (m_weight_gradient) {
            auto gradient_weight = context.allocateOutput(1, weight->get().getDtype(), weight->get().getDims());
            if (!gradient_weight) {
                context.propagate();
                return;
            }
            std::optional<std::reference_wrapper<SyclTensor>> gradient_bias;
            if (m_bias_gradient) {
                const std::array<int64_t, 1> bias_dims{weight->get().getDims()[0]};
                gradient_bias = context.allocateOutput(2, weight->get().getDtype(), bias_dims);
                if (!gradient_bias) {
                    context.propagate();
                    return;
                }
            }
            SyclConvolutionPrimitive::backward_weights(
                queue->get(),
                source->get(),
                gradient_output->get(),
                gradient_weight->get(),
                gradient_bias,
                m_geometry
            );
        }

    }

private:
    SyclConvolutionGeometry m_geometry;
    bool m_input_gradient;
    bool m_weight_gradient;
    bool m_bias_gradient;
};

} // namespace aten_xpu
