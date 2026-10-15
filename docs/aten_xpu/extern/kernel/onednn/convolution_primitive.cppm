module;

#include <oneapi/dnnl/dnnl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:onednn_convolution_primitive;

import std;
import aten_xpu_intern;
import :onednn_memory_layout;
import :onednn_post_op_attributes;
import :onednn_primitive_executor;
import :onednn_runtime;
import :onednn_convolution_geometry;

export namespace aten_xpu {

class SyclConvolutionPrimitive
{
public:
    explicit SyclConvolutionPrimitive(SyclOnednnRuntime& runtime) noexcept :
        m_runtime{runtime}
    {
    }

    sycl::event forward(
        sycl::queue& queue,
        const SyclTensor& source,
        const SyclTensor& weight,
        std::optional<std::reference_wrapper<const SyclTensor>> bias,
        SyclTensor& destination,
        const SyclConvolutionGeometry& geometry,
        const SyclPostOpAttributes& post_ops
    )
    {
        auto executor = m_runtime.make_executor(queue);
        const auto source_desc = m_layout.desc(source);
        const auto weight_desc = convolution_weight_desc(weight, geometry.getGroups());
        const auto destination_desc = m_layout.desc(destination);
        const auto bias_desc = bias ? vector_desc(bias->get()) : dnnl::memory::desc{};

        auto attributes = m_runtime.user_scratchpad_attributes();
        post_ops.apply(attributes);
        const dnnl::convolution_forward::primitive_desc primitive_desc{
            executor.getEngine(),
            dnnl::prop_kind::forward,
            dnnl::algorithm::convolution_direct,
            source_desc,
            weight_desc,
            bias_desc,
            destination_desc,
            geometry.getStride(),
            geometry.onednn_dilation(),
            geometry.getPaddingLeft(),
            geometry.getPaddingRight(),
            attributes
        };

        executor.addArgument(DNNL_ARG_SRC, source_desc, source.getData());
        executor.addArgument(DNNL_ARG_WEIGHTS, weight_desc, weight.getData());
        executor.addArgument(DNNL_ARG_DST, destination_desc, destination.getData());
        if (bias) {
            executor.addArgument(DNNL_ARG_BIAS, bias_desc, bias->get().getData());
        }
        post_ops.add_binary_arguments(executor.getEngine(), executor.getArguments());
        return executor.execute(dnnl::convolution_forward{primitive_desc}, primitive_desc);
    }

    sycl::event backward_data(
        sycl::queue& queue,
        const SyclTensor& gradient_output,
        const SyclTensor& weight,
        SyclTensor& gradient_input,
        const SyclConvolutionGeometry& geometry
    )
    {
        auto executor = m_runtime.make_executor(queue);
        const auto gradient_input_desc = m_layout.desc(gradient_input);
        const auto weight_desc = convolution_weight_desc(weight, geometry.getGroups());
        const auto gradient_output_desc = m_layout.desc(gradient_output);

        const dnnl::convolution_forward::primitive_desc hint{
            executor.getEngine(),
            dnnl::prop_kind::forward,
            dnnl::algorithm::convolution_direct,
            gradient_input_desc,
            weight_desc,
            gradient_output_desc,
            geometry.getStride(),
            geometry.onednn_dilation(),
            geometry.getPaddingLeft(),
            geometry.getPaddingRight()
        };
        const dnnl::convolution_backward_data::primitive_desc primitive_desc{
            executor.getEngine(),
            dnnl::algorithm::convolution_direct,
            gradient_input_desc,
            weight_desc,
            gradient_output_desc,
            geometry.getStride(),
            geometry.onednn_dilation(),
            geometry.getPaddingLeft(),
            geometry.getPaddingRight(),
            hint,
            m_runtime.user_scratchpad_attributes()
        };

        executor.addArgument(DNNL_ARG_DIFF_DST, gradient_output_desc, gradient_output.getData());
        executor.addArgument(DNNL_ARG_WEIGHTS, weight_desc, weight.getData());
        executor.addArgument(DNNL_ARG_DIFF_SRC, gradient_input_desc, gradient_input.getData());
        return executor.execute(dnnl::convolution_backward_data{primitive_desc}, primitive_desc);
    }

    sycl::event backward_weights(
        sycl::queue& queue,
        const SyclTensor& source,
        const SyclTensor& gradient_output,
        SyclTensor& gradient_weight,
        std::optional<std::reference_wrapper<SyclTensor>> gradient_bias,
        const SyclConvolutionGeometry& geometry
    )
    {
        auto executor = m_runtime.make_executor(queue);
        const auto source_desc = m_layout.desc(source);
        const auto gradient_weight_desc =
            convolution_weight_desc(gradient_weight, geometry.getGroups());
        const auto gradient_output_desc = m_layout.desc(gradient_output);
        const auto gradient_bias_desc =
            gradient_bias ? vector_desc(gradient_bias->get()) : dnnl::memory::desc{};

        const dnnl::convolution_forward::primitive_desc hint{
            executor.getEngine(),
            dnnl::prop_kind::forward,
            dnnl::algorithm::convolution_direct,
            source_desc,
            gradient_weight_desc,
            gradient_bias_desc,
            gradient_output_desc,
            geometry.getStride(),
            geometry.onednn_dilation(),
            geometry.getPaddingLeft(),
            geometry.getPaddingRight()
        };
        const dnnl::convolution_backward_weights::primitive_desc primitive_desc{
            executor.getEngine(),
            dnnl::algorithm::convolution_direct,
            source_desc,
            gradient_weight_desc,
            gradient_bias_desc,
            gradient_output_desc,
            geometry.getStride(),
            geometry.onednn_dilation(),
            geometry.getPaddingLeft(),
            geometry.getPaddingRight(),
            hint,
            m_runtime.user_scratchpad_attributes()
        };

        executor.addArgument(DNNL_ARG_SRC, source_desc, source.getData());
        executor.addArgument(DNNL_ARG_DIFF_DST, gradient_output_desc, gradient_output.getData());
        executor
            .addArgument(DNNL_ARG_DIFF_WEIGHTS, gradient_weight_desc, gradient_weight.getData());
        if (gradient_bias) {
            executor.addArgument(
                DNNL_ARG_DIFF_BIAS,
                gradient_bias_desc,
                gradient_bias->get().getData()
            );
        }
        return executor.execute(dnnl::convolution_backward_weights{primitive_desc}, primitive_desc);
    }

    sycl::event deconvolution(
        sycl::queue& queue,
        const SyclTensor& source,
        const SyclTensor& weight,
        std::optional<std::reference_wrapper<const SyclTensor>> bias,
        SyclTensor& destination,
        const SyclConvolutionGeometry& geometry,
        const SyclPostOpAttributes& post_ops
    )
    {
        auto executor = m_runtime.make_executor(queue);
        const auto source_desc = m_layout.desc(source);
        const auto weight_desc = deconvolution_weight_desc(weight, geometry.getGroups());
        const auto destination_desc = m_layout.desc(destination);
        const auto bias_desc = bias ? vector_desc(bias->get()) : dnnl::memory::desc{};

        auto attributes = m_runtime.user_scratchpad_attributes();
        post_ops.apply(attributes);
        const dnnl::deconvolution_forward::primitive_desc primitive_desc{
            executor.getEngine(),
            dnnl::prop_kind::forward,
            dnnl::algorithm::deconvolution_direct,
            source_desc,
            weight_desc,
            bias_desc,
            destination_desc,
            geometry.getStride(),
            geometry.onednn_dilation(),
            geometry.getPaddingLeft(),
            geometry.transposed_padding_right(),
            attributes
        };

        executor.addArgument(DNNL_ARG_SRC, source_desc, source.getData());
        executor.addArgument(DNNL_ARG_WEIGHTS, weight_desc, weight.getData());
        executor.addArgument(DNNL_ARG_DST, destination_desc, destination.getData());
        if (bias) {
            executor.addArgument(DNNL_ARG_BIAS, bias_desc, bias->get().getData());
        }
        post_ops.add_binary_arguments(executor.getEngine(), executor.getArguments());
        return executor.execute(dnnl::deconvolution_forward{primitive_desc}, primitive_desc);
    }

    dnnl::memory::desc convolution_weight_desc(const SyclTensor& weight, int64_t groups) const
    {
        const auto type =
            m_layout.data_type(weight.getDtype()).value_or(dnnl::memory::data_type::f32);
        const auto& dims = weight.getDims();
        const auto& strides = weight.getStrides();
        if (groups == 1) {
            return m_layout.desc_as(weight, type);
        }

        dnnl::memory::dims grouped_dims{groups, dims[0] / groups};
        dnnl::memory::dims grouped_strides{strides[0] * (dims[0] / groups), strides[0]};
        grouped_dims.insert(grouped_dims.end(), dims.begin() + 1, dims.end());
        grouped_strides.insert(grouped_strides.end(), strides.begin() + 1, strides.end());
        return dnnl::memory::desc{grouped_dims, type, grouped_strides};
    }

    dnnl::memory::desc deconvolution_weight_desc(const SyclTensor& weight, int64_t groups) const
    {
        const auto type =
            m_layout.data_type(weight.getDtype()).value_or(dnnl::memory::data_type::f32);
        const auto& dims = weight.getDims();
        const auto& strides = weight.getStrides();
        const auto input_per_group = dims[0] / groups;

        dnnl::memory::dims result_dims;
        dnnl::memory::dims result_strides;
        if (groups != 1) {
            result_dims.push_back(groups);
            result_strides.push_back(strides[0] * input_per_group);
        }
        result_dims.push_back(dims[1]);
        result_strides.push_back(strides[1]);
        result_dims.push_back(input_per_group);
        result_strides.push_back(strides[0]);
        result_dims.insert(result_dims.end(), dims.begin() + 2, dims.end());
        result_strides.insert(result_strides.end(), strides.begin() + 2, strides.end());
        return dnnl::memory::desc{result_dims, type, result_strides};
    }

private:
    dnnl::memory::desc vector_desc(const SyclTensor& tensor) const
    {
        const auto type =
            m_layout.data_type(tensor.getDtype()).value_or(dnnl::memory::data_type::f32);
        return dnnl::memory::desc{{tensor.element_count()}, type, dnnl::memory::format_tag::x};
    }

    SyclOnednnRuntime& m_runtime;
    SyclOnednnLayout m_layout;
};

} // namespace aten_xpu
