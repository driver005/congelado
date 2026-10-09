module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

#include <oneapi/dnnl/dnnl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:quantized_int8_matmul;

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

export namespace aten_xpu {

class SyclInt8MatmulKernel : public SyclKernel<SyclInt8MatmulKernel>
{
public:
    static constexpr std::string_view k_name = "QuantizedMatMul";
    static constexpr int k_source = 0;
    static constexpr int k_weight = 1;
    static constexpr int k_source_scale = 2;
    static constexpr int k_source_zero_point = 3;
    static constexpr int k_weight_scale = 4;
    static constexpr int k_output_scale = 5;
    static constexpr int k_output_zero_point = 6;
    static constexpr int k_bias = 7;
    static constexpr int k_other = 8;

    SyclInt8MatmulKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclInt8MatmulKernel>{ops},
        m_output_type{construction.getType("output_dtype", TF_QINT8)},
        m_weight_transposed{construction.getBool("weight_transposed", false)},
        m_unary{construction.getString("unary", "none")},
        m_algorithm{construction.getString("algorithm", "none")},
        m_binary{construction.getString("binary", "none")},
        m_other_scale{construction.getFloat("other_scale", 1.0F)},
        m_other_zero_point{construction.getInt64("other_zero_point", 0)}
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
        auto other = optional_input(context, k_other);

        const auto& source_dims = source->get().getDims();
        const int64_t inner = source_dims.back();
        const int64_t rows = source->get().element_count() / std::max<int64_t>(inner, 1);
        const auto& weight_dims = weight->get().getDims();
        const int64_t columns = m_weight_transposed ? weight_dims[0] : weight_dims[1];

        m_scratch_dims.assign(source_dims.begin(), source_dims.end());
        m_scratch_dims.back() = columns;
        auto output = context.allocateOutput(0, m_output_type, m_scratch_dims);
        if (!output) {
            context.propagate();
            return;
        }

        SyclPostOpAttributes post_ops;
        const auto scalars = std::span<const float>{m_scalars}.first(m_scalar_count);
        if (!SyclFusion::add_binary_then_unary(
                post_ops,
                m_binary,
                m_other_scale,
                m_other_zero_point,
                other ? std::optional<std::reference_wrapper<const SyclTensor>>{other->get()}
                      : std::nullopt,
                m_unary,
                scalars,
                m_algorithm
            )) {
            context.fail(TF_INVALID_ARGUMENT, "unsupported quantized matmul post op");
            return;
        }

        SyclQuantization quantization;
        quantization.setSourceScale(source_scale->get());
        quantization.setSourceZeroPoint(source_zero_point->get());
        quantization.setWeightScale(weight_scale->get(), 1 << 1);
        if (output_scale) {
            quantization.setDestinationScale(output_scale->get());
        }
        if (output_zero_point) {
            quantization.setDestinationZeroPoint(output_zero_point->get());
        }

        const dnnl::memory::desc source_desc{
            {rows, inner},
            SyclOnednnLayout::data_type(source->get().getDtype()).value(),
            dnnl::memory::format_tag::ab
        };
        const auto weight_type = SyclOnednnLayout::data_type(weight->get().getDtype()).value();
        const dnnl::memory::desc weight_desc =
            m_weight_transposed
                ? dnnl::memory::desc{{inner, columns}, weight_type, dnnl::memory::format_tag::ba}
                : dnnl::memory::desc{{inner, columns}, weight_type, dnnl::memory::format_tag::ab};
        const dnnl::memory::desc output_desc{
            {rows, columns},
            SyclOnednnLayout::data_type(m_output_type).value(),
            dnnl::memory::format_tag::ab
        };
        const auto bias_desc =
            bias
                ? dnnl::memory::
                      desc{{1, columns}, SyclOnednnLayout::data_type(bias->get().getDtype()).value(), dnnl::memory::format_tag::ab}
                : dnnl::memory::desc{};

        auto attributes = SyclPrimitiveExecutor::user_scratchpad_attributes();
        quantization.apply(attributes);
        post_ops.apply(attributes);

        SyclPrimitiveExecutor executor{queue->get()};
        const auto primitive_desc =
            bias
                ? dnnl::matmul::
                      primitive_desc{executor.getEngine(), source_desc, weight_desc, bias_desc, output_desc, attributes}
                : dnnl::matmul::primitive_desc{
                      executor.getEngine(),
                      source_desc,
                      weight_desc,
                      output_desc,
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
        executor.execute(dnnl::matmul{primitive_desc}, primitive_desc);
    }

private:
    static std::optional<std::reference_wrapper<SyclTensor>>
    optional_input(SyclKernelContext& context, int index)
    {
        return context.getInputCount() > index ? context.getInput(index) : std::nullopt;
    }

    TFDataTypeEnum m_output_type;
    bool m_weight_transposed;
    std::string m_unary;
    std::string m_algorithm;
    std::string m_binary;
    float m_other_scale;
    int64_t m_other_zero_point;
    std::array<float, 4> m_scalars{};
    std::size_t m_scalar_count{0};
    std::vector<int64_t> m_scratch_dims;
};

} // namespace aten_xpu
