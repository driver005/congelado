// SYCL reference plugin — per-tensor int8 quantized matmul via oneDNN.
//
// Not built by Bazel (docs/ only). Replaces mkldnn/detail/QMatmul.cpp's quantized_matmul for the
// per-tensor-quantization case only (mask=0 for src/weight/dst) — QMatmul.cpp also supports
// per-channel weight scale (mask 1<<1) and bias broadcasting (broadcast_bias2D/3D), neither
// ported here. Activation and output are s8 (symmetric); weight is s8.

module;

#include "include/c/extern/kernel/builder.h"
#include "docs/aten_xpu/sycl_backend/kernels/kernel_context.h"
#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>

export module sycl_backend:kernels_int8_matmul;

import std;
import :tensor;
import :stream;
import :engine_cache;

export namespace sycl_backend::kernels {

class Int8MatmulKernel
{
public:
    Int8MatmulKernel() = delete;

    // Inputs 0/1: activation, weight (both s8, [M,K]/[K,N]). Inputs 2/3: activation scale/zero
    // point (f32 scalar / s32 scalar). Input 4: weight scale (f32 scalar, per-tensor). Inputs
    // 5/6: output scale/zero point. Output 0: [M,N], s8.
    static void compute(void* plugin_data, TF_OpKernelContext* raw_context) noexcept
    {
        (void)plugin_data;

        KernelContextView ctx{raw_context};
        TF_Status status{};

        auto* activation_handle = ctx.get_input(0, &status);
        auto* weight_handle = ctx.get_input(1, &status);
        auto* activation_scale_handle = ctx.get_input(2, &status);
        auto* activation_zp_handle = ctx.get_input(3, &status);
        auto* weight_scale_handle = ctx.get_input(4, &status);
        auto* output_scale_handle = ctx.get_input(5, &status);
        auto* output_zp_handle = ctx.get_input(6, &status);
        if (activation_handle == nullptr || weight_handle == nullptr ||
            activation_scale_handle == nullptr || activation_zp_handle == nullptr ||
            weight_scale_handle == nullptr || output_scale_handle == nullptr ||
            output_zp_handle == nullptr)
        {
            ctx.fail(&status);
            return;
        }

        auto* activation = static_cast<SyclTensor*>(activation_handle->plugin_data);
        auto* weight = static_cast<SyclTensor*>(weight_handle->plugin_data);

        const std::vector<int64_t> activation_dims = shape_of(*activation);
        const std::vector<int64_t> weight_dims = shape_of(*weight);
        const int64_t m = activation_dims[0];
        const int64_t n = weight_dims[1];

        const std::vector<int64_t> dst_dims{m, n};
        auto* output_handle = ctx.allocate_output(
            0,
            TF_QINT8,
            dst_dims.data(),
            static_cast<int>(dst_dims.size()),
            static_cast<std::size_t>(m * n),
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

        run_int8_matmul(
            *stream,
            m,
            activation_dims[1],
            n,
            raw_data(*activation),
            raw_data(*weight),
            raw_data(*static_cast<SyclTensor*>(activation_scale_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(activation_zp_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(weight_scale_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(output_scale_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(output_zp_handle->plugin_data)),
            raw_data(*output)
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

    static void run_int8_matmul(
        SyclStream& stream,
        int64_t m,
        int64_t k,
        int64_t n,
        void* activation_data,
        void* weight_data,
        void* activation_scale_data,
        void* activation_zp_data,
        void* weight_scale_data,
        void* output_scale_data,
        void* output_zp_data,
        void* output_data
    )
    {
        sycl::queue& queue = stream.get_native_queue();
        dnnl::engine& engine =
            EngineCache::instance().get_engine(queue.get_device(), queue.get_context());
        dnnl::stream& dnnl_stream = EngineCache::instance().get_stream(engine, queue);

        dnnl::memory::desc activation_md{{m, k}, dnnl::memory::data_type::s8, dnnl::memory::format_tag::ab};
        dnnl::memory::desc weight_md{{k, n}, dnnl::memory::data_type::s8, dnnl::memory::format_tag::ab};
        dnnl::memory::desc dst_md{{m, n}, dnnl::memory::data_type::s8, dnnl::memory::format_tag::ab};
        dnnl::memory::desc scalar_md{{1}, dnnl::memory::data_type::f32, dnnl::memory::format_tag::x};
        dnnl::memory::desc zp_md{{1}, dnnl::memory::data_type::s32, dnnl::memory::format_tag::x};

        dnnl::primitive_attr attributes;
        attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);
        // Per-tensor scale/zero point (mask = 0) for src, weight and dst.
        attributes.set_scales_mask(DNNL_ARG_SRC, 0);
        attributes.set_zero_points_mask(DNNL_ARG_SRC, 0);
        attributes.set_scales_mask(DNNL_ARG_WEIGHTS, 0);
        attributes.set_scales_mask(DNNL_ARG_DST, 0);
        attributes.set_zero_points_mask(DNNL_ARG_DST, 0);

        dnnl::matmul::primitive_desc primitive_desc{engine, activation_md, weight_md, dst_md, attributes};
        dnnl::matmul matmul{primitive_desc};

        std::unordered_map<int, dnnl::memory> arguments;
        arguments.emplace(DNNL_ARG_SRC, dnnl::memory{activation_md, engine, activation_data});
        arguments.emplace(DNNL_ARG_WEIGHTS, dnnl::memory{weight_md, engine, weight_data});
        arguments.emplace(DNNL_ARG_DST, dnnl::memory{dst_md, engine, output_data});
        arguments.emplace(
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_SRC,
            dnnl::memory{scalar_md, engine, activation_scale_data}
        );
        arguments.emplace(
            DNNL_ARG_ATTR_ZERO_POINTS | DNNL_ARG_SRC,
            dnnl::memory{zp_md, engine, activation_zp_data}
        );
        arguments.emplace(
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_WEIGHTS,
            dnnl::memory{scalar_md, engine, weight_scale_data}
        );
        arguments.emplace(
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_DST,
            dnnl::memory{scalar_md, engine, output_scale_data}
        );
        arguments.emplace(
            DNNL_ARG_ATTR_ZERO_POINTS | DNNL_ARG_DST,
            dnnl::memory{zp_md, engine, output_zp_data}
        );

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
