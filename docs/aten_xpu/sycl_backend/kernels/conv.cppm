// SYCL reference plugin — 2D convolution forward via oneDNN.
//
// Not built by Bazel (docs/ only). Replaces mkldnn/detail/Conv.cpp's forward path for the plain
// (no post-op fusion, no channels-last, no backward) NCHW case. stride/padding/dilation/groups
// are read once at kernel construction time (create_func) from the op's attrs, matching
// create_kernel_builder's create_func/compute_func/delete_func triple — this is the first
// kernel in this plugin that actually uses construction-time state instead of pure per-call
// dispatch.

module;

#include "include/c/extern/kernel/builder.h"
#include "docs/aten_xpu/sycl_backend/kernels/kernel_context.h"
#include "docs/aten_xpu/sycl_backend/kernels/kernel_construction_view.h"
#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>

export module sycl_backend:kernels_conv;

import std;
import :tensor;
import :stream;
import :engine_cache;

export namespace sycl_backend::kernels {

class ConvAttrs
{
public:
    int64_t stride{1};
    int64_t padding{0};
    int64_t dilation{1};
    int64_t groups{1};
    // Set by a grappler fusion pass rewriting Conv2D+BiasAdd+Relu into one Conv2D node — see
    // ../optimizer.cppm. 0 = none, 1 = relu.
    int64_t activation{0};
};

class ConvKernel
{
public:
    ConvKernel() = delete;

    static void create(TF_OpKernelConstruction* raw_construction, void** out_plugin_data) noexcept
    {
        KernelConstructionView construction{raw_construction};
        TF_Status status{};

        auto* attrs = new ConvAttrs{};
        attrs->stride = construction.get_attr_int64("stride", 1, &status);
        attrs->padding = construction.get_attr_int64("padding", 0, &status);
        attrs->dilation = construction.get_attr_int64("dilation", 1, &status);
        attrs->groups = construction.get_attr_int64("groups", 1, &status);
        attrs->activation = construction.get_attr_int64("activation", 0, &status);

        *out_plugin_data = attrs;
    }

    static void destroy(void* plugin_data) noexcept
    {
        delete static_cast<ConvAttrs*>(plugin_data);
    }

    // Input 0: source [N, C_in, H, W]. Input 1: weight [C_out, C_in/groups, kH, kW]. Input 2
    // (optional): bias [C_out]. Output 0: [N, C_out, H_out, W_out].
    static void compute(void* plugin_data, TF_OpKernelContext* raw_context) noexcept
    {
        const auto& attrs = *static_cast<ConvAttrs*>(plugin_data);
        KernelContextView ctx{raw_context};
        TF_Status status{};

        auto* source_handle = ctx.get_input(0, &status);
        auto* weight_handle = ctx.get_input(1, &status);
        if (source_handle == nullptr || weight_handle == nullptr) {
            ctx.fail(&status);
            return;
        }

        auto* source = static_cast<SyclTensor*>(source_handle->plugin_data);
        auto* weight = static_cast<SyclTensor*>(weight_handle->plugin_data);

        const std::vector<int64_t> source_dims = shape_of(*source);
        const std::vector<int64_t> weight_dims = shape_of(*weight);

        const int64_t output_height =
            (source_dims[2] + 2 * attrs.padding - attrs.dilation * (weight_dims[2] - 1) - 1) /
                attrs.stride +
            1;
        const int64_t output_width =
            (source_dims[3] + 2 * attrs.padding - attrs.dilation * (weight_dims[3] - 1) - 1) /
                attrs.stride +
            1;
        const std::vector<int64_t> dst_dims{
            source_dims[0],
            weight_dims[0],
            output_height,
            output_width
        };

        uint64_t element_count = 1;
        for (int64_t extent: dst_dims) {
            element_count *= static_cast<uint64_t>(extent);
        }

        auto* output_handle = ctx.allocate_output(
            0,
            TF_FLOAT,
            dst_dims.data(),
            static_cast<int>(dst_dims.size()),
            element_count * SyclTensor::element_size(TF_FLOAT),
            &status
        );
        if (output_handle == nullptr) {
            ctx.fail(&status);
            return;
        }
        auto* dst = static_cast<SyclTensor*>(output_handle->plugin_data);

        auto* stream_handle = ctx.get_stream(&status);
        if (stream_handle == nullptr) {
            ctx.fail(&status);
            return;
        }
        auto* stream = static_cast<SyclStream*>(stream_handle->plugin_data);

        void* bias_data = nullptr;
        std::vector<int64_t> bias_dims;
        if (ctx.num_inputs() > 2) {
            auto* bias_handle = ctx.get_input(2, &status);
            if (bias_handle != nullptr) {
                auto* bias = static_cast<SyclTensor*>(bias_handle->plugin_data);
                bias_data = raw_data(*bias);
                bias_dims = {weight_dims[0]};
            }
        }

        run_convolution(
            *stream,
            source_dims,
            raw_data(*source),
            weight_dims,
            raw_data(*weight),
            dst_dims,
            raw_data(*dst),
            bias_dims,
            bias_data,
            attrs
        );
    }

private:
    static std::vector<int64_t> shape_of(SyclTensor& tensor)
    {
        int rank = 0;
        tensor.num_dims(&rank);

        std::vector<int64_t> dims(static_cast<std::size_t>(rank));
        for (int index = 0; index < rank; ++index) {
            tensor.dim(index, &dims[static_cast<std::size_t>(index)]);
        }
        return dims;
    }

    static void* raw_data(SyclTensor& tensor)
    {
        void* data = nullptr;
        tensor.tensor_data(&data);
        return data;
    }

    static void run_convolution(
        SyclStream& stream,
        const std::vector<int64_t>& source_dims,
        void* source_data,
        const std::vector<int64_t>& weight_dims,
        void* weight_data,
        const std::vector<int64_t>& dst_dims,
        void* dst_data,
        const std::vector<int64_t>& bias_dims,
        void* bias_data,
        const ConvAttrs& attrs
    )
    {
        sycl::queue& queue = stream.get_native_queue();
        dnnl::engine& engine =
            EngineCache::instance().get_engine(queue.get_device(), queue.get_context());
        dnnl::stream& dnnl_stream = EngineCache::instance().get_stream(engine, queue);

        auto data_type = dnnl::memory::data_type::f32;
        dnnl::memory::desc source_md{source_dims, data_type, dnnl::memory::format_tag::nchw};
        dnnl::memory::desc weight_md{weight_dims, data_type, dnnl::memory::format_tag::oihw};
        dnnl::memory::desc dst_md{dst_dims, data_type, dnnl::memory::format_tag::nchw};
        dnnl::memory::desc bias_md = bias_data != nullptr
            ? dnnl::memory::desc{bias_dims, data_type, dnnl::memory::format_tag::x}
            : dnnl::memory::desc{};

        dnnl::primitive_attr attributes;
        attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);
        if (attrs.activation == 1) {
            dnnl::post_ops post_ops;
            post_ops.append_eltwise(dnnl::algorithm::eltwise_relu, 0.0F, 0.0F);
            attributes.set_post_ops(post_ops);
        }

        auto primitive_desc = dnnl::convolution_forward::primitive_desc{
            engine,
            dnnl::prop_kind::forward,
            dnnl::algorithm::convolution_direct,
            source_md,
            weight_md,
            bias_md,
            dst_md,
            dnnl::memory::dims{attrs.stride, attrs.stride},
            dnnl::memory::dims{attrs.dilation - 1, attrs.dilation - 1},
            dnnl::memory::dims{attrs.padding, attrs.padding},
            dnnl::memory::dims{attrs.padding, attrs.padding},
            attributes
        };

        dnnl::convolution_forward convolution{primitive_desc};

        std::unordered_map<int, dnnl::memory> arguments;
        arguments.emplace(DNNL_ARG_SRC, dnnl::memory{source_md, engine, source_data});
        arguments.emplace(DNNL_ARG_WEIGHTS, dnnl::memory{weight_md, engine, weight_data});
        arguments.emplace(DNNL_ARG_DST, dnnl::memory{dst_md, engine, dst_data});
        if (bias_data != nullptr) {
            arguments.emplace(DNNL_ARG_BIAS, dnnl::memory{bias_md, engine, bias_data});
        }

        const std::size_t scratchpad_size = primitive_desc.scratchpad_desc().get_size();
        void* scratchpad_data =
            scratchpad_size == 0 ? nullptr : sycl::malloc_device(scratchpad_size, queue);
        if (scratchpad_data != nullptr) {
            arguments.emplace(
                DNNL_ARG_SCRATCHPAD,
                dnnl::memory{primitive_desc.scratchpad_desc(), engine, scratchpad_data}
            );
        }

        dnnl::sycl_interop::execute(convolution, dnnl_stream, arguments);

        if (scratchpad_data != nullptr) {
            queue.wait();
            sycl::free(scratchpad_data, queue);
        }
    }
};

} // namespace sycl_backend::kernels
