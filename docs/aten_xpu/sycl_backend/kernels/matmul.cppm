// SYCL reference plugin — addmm / bmm via oneDNN matmul.
//
// Not built by Bazel (docs/ only). Replaces mkldnn/detail/Matmul.cpp's core matmul path for the
// plain (no binary/TF32/deterministic attr) case: contiguous row-major operands, an optional
// bias, one dnnl::matmul primitive, and an optional fused ReLU post-op (the "activation" attr,
// set by a grappler fusion pass — see ../optimizer.cppm). Attr.h's fuller post-op construction
// (arbitrary binary ops, more eltwise kinds) and FusionUtils are not ported.

module;

#include "docs/aten_xpu/sycl_backend/kernels/kernel_construction_view.h"
#include "docs/aten_xpu/sycl_backend/kernels/kernel_context.h"
#include "include/c/extern/kernel/builder.h"

#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>

export module sycl_backend:kernels_matmul;

import std;
import :tensor;
import :stream;
import :engine_cache;

export namespace sycl_backend::kernels {

class MatmulAttrs
{
public:
    // Set by a grappler fusion pass rewriting Matmul+BiasAdd+Relu into one Matmul node — see
    // ../optimizer.cppm. 0 = none, 1 = relu.
    int64_t activation{0};
};

class MatmulKernel
{
public:
    MatmulKernel() = delete;

    static void create(TF_OpKernelConstruction* raw_construction, void** out_plugin_data) noexcept
    {
        KernelConstructionView construction{raw_construction};
        TF_Status status{};

        auto* attrs = new MatmulAttrs{};
        attrs->activation = construction.get_attr_int64("activation", 0, &status);

        *out_plugin_data = attrs;
    }

    static void destroy(void* plugin_data) noexcept
    {
        delete static_cast<MatmulAttrs*>(plugin_data);
    }

    // ctx has inputs [mat1, mat2] and, if with_bias, [mat1, mat2, bias]; output 0 is the result.
    // Mirrors mkldnn/Blas.cpp's addmm_out_xpu / bmm_out_xpu after alpha/beta have already been
    // folded in by the caller (alpha == 1, beta == 0 or 1 here).
    static void compute(void* plugin_data, TF_OpKernelContext* raw_context, bool with_bias) noexcept
    {
        const int64_t activation =
            plugin_data == nullptr ? 0 : static_cast<MatmulAttrs*>(plugin_data)->activation;

        KernelContextView ctx{raw_context};
        TF_Status status{};

        auto* mat1_handle = ctx.get_input(0, &status);
        auto* mat2_handle = ctx.get_input(1, &status);
        if (mat1_handle == nullptr || mat2_handle == nullptr) {
            ctx.fail(&status);
            return;
        }

        auto* mat1 = static_cast<SyclTensor*>(mat1_handle->plugin_data);
        auto* mat2 = static_cast<SyclTensor*>(mat2_handle->plugin_data);

        std::vector<int64_t> dst_dims{shape_of(*mat1).front(), shape_of(*mat2).back()};
        auto* output_handle = ctx.allocate_output(
            0,
            TF_FLOAT,
            dst_dims.data(),
            static_cast<int>(dst_dims.size()),
            static_cast<size_t>(dst_dims[0] * dst_dims[1]) * SyclTensor::element_size(TF_FLOAT),
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
        if (with_bias) {
            auto* bias_handle = ctx.get_input(2, &status);
            if (bias_handle == nullptr) {
                ctx.fail(&status);
                return;
            }
            auto* bias = static_cast<SyclTensor*>(bias_handle->plugin_data);
            bias_data = raw_data(*bias);
            bias_dims = shape_of(*bias);
        }

        run_matmul(
            *stream,
            shape_of(*mat1),
            raw_data(*mat1),
            shape_of(*mat2),
            raw_data(*mat2),
            dst_dims,
            raw_data(*dst),
            bias_dims,
            bias_data,
            activation
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

    static void run_matmul(
        SyclStream& stream,
        const std::vector<int64_t>& m1_dims,
        void* m1_data,
        const std::vector<int64_t>& m2_dims,
        void* m2_data,
        const std::vector<int64_t>& dst_dims,
        void* dst_data,
        const std::vector<int64_t>& bias_dims,
        void* bias_data,
        int64_t activation
    )
    {
        sycl::queue& queue = stream.get_native_queue();
        dnnl::engine& engine =
            EngineCache::instance().get_engine(queue.get_device(), queue.get_context());
        dnnl::stream& dnnl_stream = EngineCache::instance().get_stream(engine, queue);

        auto data_type = dnnl::memory::data_type::f32;
        dnnl::memory::desc m1_md{m1_dims, data_type, dnnl::memory::format_tag::ab};
        dnnl::memory::desc m2_md{m2_dims, data_type, dnnl::memory::format_tag::ab};
        dnnl::memory::desc dst_md{dst_dims, data_type, dnnl::memory::format_tag::ab};

        dnnl::primitive_attr attributes;
        attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);
        if (activation == 1) {
            dnnl::post_ops post_ops;
            post_ops.append_eltwise(dnnl::algorithm::eltwise_relu, 0.0F, 0.0F);
            attributes.set_post_ops(post_ops);
        }

        dnnl::matmul::primitive_desc primitive_desc =
            bias_data != nullptr
                ? dnnl::matmul::
                      primitive_desc{engine, m1_md, m2_md, dnnl::memory::desc{bias_dims, data_type, dnnl::memory::format_tag::ab}, dst_md, attributes}
                : dnnl::matmul::primitive_desc{engine, m1_md, m2_md, dst_md, attributes};

        dnnl::matmul matmul{primitive_desc};

        std::unordered_map<int, dnnl::memory> arguments;
        arguments.emplace(DNNL_ARG_SRC, dnnl::memory{m1_md, engine, m1_data});
        arguments.emplace(DNNL_ARG_WEIGHTS, dnnl::memory{m2_md, engine, m2_data});
        arguments.emplace(DNNL_ARG_DST, dnnl::memory{dst_md, engine, dst_data});

        if (bias_data != nullptr) {
            arguments.emplace(
                DNNL_ARG_BIAS,
                dnnl::memory{
                    dnnl::memory::desc{bias_dims, data_type, dnnl::memory::format_tag::ab},
                    engine,
                    bias_data
                }
            );
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
