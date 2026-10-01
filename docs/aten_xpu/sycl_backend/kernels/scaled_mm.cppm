// SYCL reference plugin — tensor-wise scaled matmul (out = (A @ B) * scale_a * scale_b [+ bias])
// via oneDNN.
//
// Not built by Bazel (docs/ only). Replaces mkldnn/ScaledBlas.cpp's _scaled_gemm /
// _scaled_mm_xpu for ScalingType::TensorWise only — RowWise and the four BlockWise (fp8
// microscaling) recipes core/XPUScaledBlas.cpp's recipe-compatibility table allows are not
// ported; A/B are read as s8 here (a real port would take the dtype from the tensor and support
// f8_e4m3/f8_e5m2 too).

module;

#include "docs/aten_xpu/sycl_backend/kernels/kernel_context.h"
#include "include/c/extern/kernel/builder.h"

#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>

export module sycl_backend:kernels_scaled_mm;

import std;
import :tensor;
import :stream;
import :engine_cache;

export namespace sycl_backend::kernels {

class ScaledMmKernel
{
public:
    ScaledMmKernel() = delete;

    // Inputs 0/1: A [M,K], B [K,N] (both s8). Inputs 2/3: per-tensor scale_a, scale_b (f32
    // scalar). Input 4 (optional): bias [N] (f32). Output 0: [M,N], f32.
    static void compute(void* plugin_data, TF_OpKernelContext* raw_context) noexcept
    {
        (void)plugin_data;

        KernelContextView ctx{raw_context};
        TF_Status status{};

        auto* a_handle = ctx.get_input(0, &status);
        auto* b_handle = ctx.get_input(1, &status);
        auto* scale_a_handle = ctx.get_input(2, &status);
        auto* scale_b_handle = ctx.get_input(3, &status);
        if (a_handle == nullptr || b_handle == nullptr || scale_a_handle == nullptr ||
            scale_b_handle == nullptr) {
            ctx.fail(&status);
            return;
        }

        auto* a = static_cast<SyclTensor*>(a_handle->plugin_data);
        auto* b = static_cast<SyclTensor*>(b_handle->plugin_data);
        const std::vector<int64_t> a_dims = shape_of(*a);
        const std::vector<int64_t> b_dims = shape_of(*b);
        const int64_t m = a_dims[0];
        const int64_t k = a_dims[1];
        const int64_t n = b_dims[1];

        const std::vector<int64_t> dst_dims{m, n};
        auto* output_handle = ctx.allocate_output(
            0,
            TF_FLOAT,
            dst_dims.data(),
            static_cast<int>(dst_dims.size()),
            static_cast<std::size_t>(m * n) * SyclTensor::element_size(TF_FLOAT),
            &status
        );
        if (output_handle == nullptr) {
            ctx.fail(&status);
            return;
        }

        auto* stream_handle = ctx.get_stream(&status);
        if (stream_handle == nullptr) {
            ctx.fail(&status);
            return;
        }
        auto* stream = static_cast<SyclStream*>(stream_handle->plugin_data);

        void* bias_data = nullptr;
        if (ctx.num_inputs() > 4) {
            auto* bias_handle = ctx.get_input(4, &status);
            if (bias_handle != nullptr) {
                bias_data = raw_data(*static_cast<SyclTensor*>(bias_handle->plugin_data));
            }
        }

        run_scaled_matmul(
            *stream,
            m,
            k,
            n,
            raw_data(*a),
            raw_data(*b),
            raw_data(*static_cast<SyclTensor*>(scale_a_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(scale_b_handle->plugin_data)),
            bias_data,
            raw_data(*static_cast<SyclTensor*>(output_handle->plugin_data))
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

    static void run_scaled_matmul(
        SyclStream& stream,
        int64_t m,
        int64_t k,
        int64_t n,
        void* a_data,
        void* b_data,
        void* scale_a_data,
        void* scale_b_data,
        void* bias_data,
        void* output_data
    )
    {
        sycl::queue& queue = stream.get_native_queue();
        dnnl::engine& engine =
            EngineCache::instance().get_engine(queue.get_device(), queue.get_context());
        dnnl::stream& dnnl_stream = EngineCache::instance().get_stream(engine, queue);

        dnnl::memory::desc a_md{{m, k}, dnnl::memory::data_type::s8, dnnl::memory::format_tag::ab};
        dnnl::memory::desc b_md{{k, n}, dnnl::memory::data_type::s8, dnnl::memory::format_tag::ab};
        dnnl::memory::desc dst_md{
            {m, n},
            dnnl::memory::data_type::f32,
            dnnl::memory::format_tag::ab
        };
        dnnl::memory::desc bias_md =
            bias_data != nullptr
                ? dnnl::memory::desc{{n}, dnnl::memory::data_type::f32, dnnl::memory::format_tag::x}
                : dnnl::memory::desc{};
        dnnl::memory::desc scalar_md{
            {1},
            dnnl::memory::data_type::f32,
            dnnl::memory::format_tag::x
        };

        dnnl::primitive_attr attributes;
        attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);
        attributes.set_scales_mask(DNNL_ARG_SRC, 0);
        attributes.set_scales_mask(DNNL_ARG_WEIGHTS, 0);

        dnnl::matmul::primitive_desc primitive_desc =
            bias_data != nullptr
                ? dnnl::matmul::primitive_desc{engine, a_md, b_md, bias_md, dst_md, attributes}
                : dnnl::matmul::primitive_desc{engine, a_md, b_md, dst_md, attributes};
        dnnl::matmul matmul{primitive_desc};

        std::unordered_map<int, dnnl::memory> arguments;
        arguments.emplace(DNNL_ARG_SRC, dnnl::memory{a_md, engine, a_data});
        arguments.emplace(DNNL_ARG_WEIGHTS, dnnl::memory{b_md, engine, b_data});
        arguments.emplace(DNNL_ARG_DST, dnnl::memory{dst_md, engine, output_data});
        arguments.emplace(
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_SRC,
            dnnl::memory{scalar_md, engine, scale_a_data}
        );
        arguments.emplace(
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_WEIGHTS,
            dnnl::memory{scalar_md, engine, scale_b_data}
        );
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
