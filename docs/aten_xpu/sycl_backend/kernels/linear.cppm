// SYCL reference plugin — linear (y = x @ weight^T + bias) via oneDNN matmul.
//
// Not built by Bazel (docs/ only). Replaces mkldnn/Linear.cpp's linear_pointwise for the plain
// (no post-op fusion) 2D case. weight is stored [out_features, in_features] row-major, same as
// nn.Linear.weight; matmul.cppm's matmul wants a [in_features, out_features] operand. Rather
// than materialize a transposed copy, this builds the oneDNN memory::desc for weight with dims
// swapped and strides {1, in_features} — the same "read transposed via strides" trick
// mkldnn/Linear.cpp gets from at::native::onednn::matmul's m2_trans handling.

module;

#include "include/c/extern/kernel/builder.h"
#include "docs/aten_xpu/sycl_backend/kernels/kernel_context.h"
#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>

export module sycl_backend:kernels_linear;

import std;
import :tensor;
import :stream;
import :engine_cache;

export namespace sycl_backend::kernels {

class LinearKernel
{
public:
    LinearKernel() = delete;

    // Input 0: x [..., in_features] (only the 2D [M, in_features] case is handled). Input 1:
    // weight [out_features, in_features]. Input 2 (optional): bias [out_features]. Output 0:
    // [M, out_features].
    static void compute(void* plugin_data, TF_OpKernelContext* raw_context) noexcept
    {
        (void)plugin_data;

        KernelContextView ctx{raw_context};
        TF_Status status{};

        auto* input_handle = ctx.get_input(0, &status);
        auto* weight_handle = ctx.get_input(1, &status);
        if (input_handle == nullptr || weight_handle == nullptr) {
            ctx.fail(&status);
            return;
        }

        auto* input = static_cast<SyclTensor*>(input_handle->plugin_data);
        auto* weight = static_cast<SyclTensor*>(weight_handle->plugin_data);

        const std::vector<int64_t> input_dims = shape_of(*input);
        const std::vector<int64_t> weight_dims = shape_of(*weight);
        const int64_t rows = input_dims[0];
        const int64_t in_features = input_dims[1];
        const int64_t out_features = weight_dims[0];

        const std::vector<int64_t> dst_dims{rows, out_features};
        auto* output_handle = ctx.allocate_output(
            0,
            TF_FLOAT,
            dst_dims.data(),
            static_cast<int>(dst_dims.size()),
            static_cast<std::size_t>(rows * out_features) * SyclTensor::element_size(TF_FLOAT),
            &status
        );
        if (output_handle == nullptr) {
            ctx.fail(&status);
            return;
        }
        auto* output = static_cast<SyclTensor*>(output_handle->plugin_data);

        auto* stream_handle = ctx.get_stream(&status);
        if (stream_handle == nullptr) {
            ctx.fail(&status);
            return;
        }
        auto* stream = static_cast<SyclStream*>(stream_handle->plugin_data);

        void* bias_data = nullptr;
        if (ctx.num_inputs() > 2) {
            auto* bias_handle = ctx.get_input(2, &status);
            if (bias_handle != nullptr) {
                bias_data = raw_data(*static_cast<SyclTensor*>(bias_handle->plugin_data));
            }
        }

        run_linear(
            *stream,
            rows,
            in_features,
            out_features,
            raw_data(*input),
            raw_data(*weight),
            raw_data(*output),
            bias_data
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

    static void run_linear(
        SyclStream& stream,
        int64_t rows,
        int64_t in_features,
        int64_t out_features,
        void* input_data,
        void* weight_data,
        void* output_data,
        void* bias_data
    )
    {
        sycl::queue& queue = stream.get_native_queue();
        dnnl::engine& engine =
            EngineCache::instance().get_engine(queue.get_device(), queue.get_context());
        dnnl::stream& dnnl_stream = EngineCache::instance().get_stream(engine, queue);

        auto data_type = dnnl::memory::data_type::f32;
        dnnl::memory::desc input_md{{rows, in_features}, data_type, dnnl::memory::format_tag::ab};
        // weight is physically [out_features, in_features] row-major; read as [in_features,
        // out_features] via strides {1, in_features} — see the file-level note.
        dnnl::memory::desc weight_md{
            {in_features, out_features},
            data_type,
            dnnl::memory::dims{1, in_features}
        };
        dnnl::memory::desc dst_md{{rows, out_features}, data_type, dnnl::memory::format_tag::ab};
        dnnl::memory::desc bias_md = bias_data != nullptr
            ? dnnl::memory::desc{{out_features}, data_type, dnnl::memory::format_tag::x}
            : dnnl::memory::desc{};

        dnnl::primitive_attr attributes;
        attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);

        dnnl::matmul::primitive_desc primitive_desc = bias_data != nullptr
            ? dnnl::matmul::primitive_desc{engine, input_md, weight_md, bias_md, dst_md, attributes}
            : dnnl::matmul::primitive_desc{engine, input_md, weight_md, dst_md, attributes};

        dnnl::matmul matmul{primitive_desc};

        std::unordered_map<int, dnnl::memory> arguments;
        arguments.emplace(DNNL_ARG_SRC, dnnl::memory{input_md, engine, input_data});
        arguments.emplace(DNNL_ARG_WEIGHTS, dnnl::memory{weight_md, engine, weight_data});
        arguments.emplace(DNNL_ARG_DST, dnnl::memory{dst_md, engine, output_data});
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

        dnnl::sycl_interop::execute(matmul, dnnl_stream, arguments);

        if (scratchpad_data != nullptr) {
            queue.wait();
            sycl::free(scratchpad_data, queue);
        }
    }
};

} // namespace sycl_backend::kernels
