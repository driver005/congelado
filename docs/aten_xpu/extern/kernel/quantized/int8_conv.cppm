module;

#include "include/c/intern/datatype.h"

#include <oneapi/dnnl/dnnl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:quantized_int8_conv;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;
import :onednn_memory_layout;
import :onednn_post_op_attributes;
import :onednn_fusion;
import :onednn_primitive_executor;
import :onednn_quantization;
import :onednn_convolution_geometry;
import :onednn_convolution_primitive;

export namespace aten_xpu {

class SyclInt8ConvolutionKernel : public SyclKernel<SyclInt8ConvolutionKernel>
{
public:
    static constexpr std::string_view k_name = "QuantizedConv2D";
    static constexpr int k_source = 0;
    static constexpr int k_weight = 1;
    static constexpr int k_source_scale = 2;
    static constexpr int k_source_zero_point = 3;
    static constexpr int k_weight_scale = 4;
    static constexpr int k_output_scale = 5;
    static constexpr int k_output_zero_point = 6;
    static constexpr int k_bias = 7;
    static constexpr int k_accumulator = 8;

    SyclInt8ConvolutionKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclInt8ConvolutionKernel>{ops},
        m_geometry{SyclConvolutionGeometry::from_construction(construction)},
        m_output_type{construction.getType("output_dtype", TF_QINT8)},
        m_unary{construction.getString("unary", "none")},
        m_algorithm{construction.getString("algorithm", "none")},
        m_binary{construction.getString("binary", "none")},
        m_accumulator_scale{construction.getFloat("accumulator_scale", 1.0F)},
        m_accumulator_zero_point{construction.getInt64("accumulator_zero_point", 0)}
    {

        m_scalar_count = construction.getFloatList("scalars", m_scalars);

    }

    void compute(SyclKernelContext& context)
    {

        auto source = context.getInput(k_source);
        auto weight = context.getInput(k_weight);
        auto source_scale = context.getInput(k_source_scale);
        auto source_zero_point = context.getInput(k_source_zero_point);
        auto weight_scale = context.getInput(k_weight_scale);
        auto queue = context.getQueue();
        if (!source || !weight || !source_scale || !source_zero_point || !weight_scale || !queue) {
            context.propagate();
            return;
        }
        auto output_scale = optional_input(context, k_output_scale);
        auto output_zero_point = optional_input(context, k_output_zero_point);
        auto bias = optional_input(context, k_bias);
        auto accumulator = optional_input(context, k_accumulator);

        m_geometry.setSpatialRank(source->get().getDims().size() - 2);
        const auto dims = m_geometry.output_dims(source->get().getDims(), weight->get().getDims());
        m_scratch_dims.assign(dims.begin(), dims.end());
        auto output = context.allocateOutput(0, m_output_type, m_scratch_dims);
        if (!output) {
            context.propagate();
            return;
        }
        if (accumulator && m_binary == "sum") {
            queue->get().memcpy(output->get().getData(), accumulator->get().getData(), accumulator->get().getByteSize());
        }

        SyclPostOpAttributes post_ops;
        const auto scalars = std::span<const float>{m_scalars}.first(m_scalar_count);
        if (!SyclFusion::add_binary_then_unary(
                post_ops,
                m_binary,
                m_accumulator_scale,
                m_accumulator_zero_point,
                accumulator ? std::optional<std::reference_wrapper<const SyclTensor>>{accumulator->get()} : std::nullopt,
                m_unary,
                scalars,
                m_algorithm
            ))
        {
            context.fail(TF_INVALID_ARGUMENT, "unsupported quantized convolution post op");
            return;
        }

        SyclQuantization quantization;
        quantization.setSourceScale(source_scale->get());
        quantization.setSourceZeroPoint(source_zero_point->get());
        quantization.setWeightScale(weight_scale->get(), m_geometry.getGroups() > 1 ? 0b11 : 0b1);
        if (output_scale) {
            quantization.setDestinationScale(output_scale->get());
        }
        if (output_zero_point) {
            quantization.setDestinationZeroPoint(output_zero_point->get());
        }

        SyclPrimitiveExecutor executor{queue->get()};
        const auto source_desc = SyclOnednnLayout::desc(source->get());
        const auto weight_desc = SyclConvolutionPrimitive::convolution_weight_desc(weight->get(), m_geometry.getGroups());
        const auto output_desc = SyclOnednnLayout::desc(output->get());
        const auto bias_desc = bias ? dnnl::memory::desc{{bias->get().element_count()}, dnnl::memory::data_type::f32, dnnl::memory::format_tag::x}
                                    : dnnl::memory::desc{};

        auto attributes = SyclPrimitiveExecutor::user_scratchpad_attributes();
        quantization.apply(attributes);
        post_ops.apply(attributes);
        const dnnl::convolution_forward::primitive_desc primitive_desc{
            executor.getEngine(),
            dnnl::prop_kind::forward_inference,
            dnnl::algorithm::convolution_direct,
            source_desc,
            weight_desc,
            bias_desc,
            output_desc,
            m_geometry.getStride(),
            m_geometry.onednn_dilation(),
            m_geometry.getPaddingLeft(),
            m_geometry.getPaddingRight(),
            attributes
        };

        executor.addArgument(DNNL_ARG_SRC, source_desc, source->get().getData());
        executor.addArgument(DNNL_ARG_WEIGHTS, weight_desc, weight->get().getData());
        executor.addArgument(DNNL_ARG_DST, output_desc, output->get().getData());
        if (bias) {
            executor.addArgument(DNNL_ARG_BIAS, bias_desc, bias->get().getData());
        }
        quantization.add_arguments(executor);
        post_ops.add_binary_arguments(executor.getEngine(), executor.getArguments());
        executor.execute(dnnl::convolution_forward{primitive_desc}, primitive_desc);

    }

private:
    static std::optional<std::reference_wrapper<SyclTensor>> optional_input(SyclKernelContext& context, int index)
    {

        return context.getInputCount() > index ? context.getInput(index) : std::nullopt;

    }

    SyclConvolutionGeometry m_geometry;
    TFDataTypeEnum m_output_type;
    std::string m_unary;
    std::string m_algorithm;
    std::string m_binary;
    float m_accumulator_scale;
    int64_t m_accumulator_zero_point;
    std::array<float, 4> m_scalars{};
    std::size_t m_scalar_count{0};
    std::vector<int64_t> m_scratch_dims;
};

} // namespace aten_xpu
