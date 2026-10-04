// SYCL reference plugin — weight-only int4 quantized matmul via oneDNN.
//
// Not built by Bazel (docs/ only). Replaces mkldnn/detail/WoQMatmul.cpp's woq_matmul_int4_impl:
// activation stays f32 (real ATen supports f16/bf16; this reference tensor type only tracks
// TF_FLOAT-sized elements cleanly, so f32 is what this port uses), weight is packed 2 values per
// byte (u4), one (scale, zero_point) pair per group of `group_size` K-elements per output
// column — the standard GPTQ-style layout. No DnnlExt primitive cache (LRUCache.h) — this port
// goes through EngineCache only, so a new dnnl::matmul::primitive_desc is built per call.

module;

#include "docs/aten_xpu/sycl_backend/kernels/kernel_construction_view.h"
#include "docs/aten_xpu/sycl_backend/kernels/kernel_context.h"
#include "include/c/extern/kernel/builder.h"

#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>

export module sycl_backend:kernels_woq_matmul;

import std;
import :tensor;
import :stream;
import :engine_cache;

export namespace sycl_backend::kernels {

class WoqMatmulAttrs
{
public:
    int64_t group_size{128};
};

class WoqMatmulKernel
{
public:
    WoqMatmulKernel() = delete;

    static void create(TF_OpKernelConstruction* raw_construction, void** out_plugin_data) noexcept
    {
        KernelConstructionView construction{raw_construction};
        TF_Status status{};

        auto* attrs = new WoqMatmulAttrs{};
        attrs->group_size = construction.get_attr_int64("group_size", 128, &status);

        *out_plugin_data = attrs;
    }

    static void destroy(void* plugin_data) noexcept
    {
        delete static_cast<WoqMatmulAttrs*>(plugin_data);
    }

    // Input 0: activation [M, K] (f32). Input 1: packed weight, two u4 values per byte, stored
    // as [K/2, N] row-major (matches WoQMatmul.cpp's m2_usr_dims = {compressed_k, n}). Input 2:
    // per-group scale [K/group_size, N] (f32). Input 3: per-group zero point [K/group_size, N]
    // (s8, one byte per group/column — not packed). Output 0: [M, N].
    static void compute(void* plugin_data, TF_OpKernelContext* raw_context) noexcept
    {
        const auto& attrs = *static_cast<WoqMatmulAttrs*>(plugin_data);
        KernelContextView ctx{raw_context};
        TF_Status status{};

        auto* activation_handle = ctx.get_input(0, &status);
        auto* weight_handle = ctx.get_input(1, &status);
        auto* scale_handle = ctx.get_input(2, &status);
        auto* zero_point_handle = ctx.get_input(3, &status);
        if (activation_handle == nullptr || weight_handle == nullptr || scale_handle == nullptr ||
            zero_point_handle == nullptr) {
            ctx.fail(&status);
            return;
        }

        auto* activation = static_cast<SyclTensor*>(activation_handle->plugin_data);
        auto* weight = static_cast<SyclTensor*>(weight_handle->plugin_data);
        auto* scale = static_cast<SyclTensor*>(scale_handle->plugin_data);
        auto* zero_point = static_cast<SyclTensor*>(zero_point_handle->plugin_data);

        const std::vector<int64_t> activation_dims = shape_of(*activation);
        const int64_t m = activation_dims[0];
        const int64_t k = activation_dims[1];
        const std::vector<int64_t> weight_dims = shape_of(*weight);
        const int64_t n = weight_dims[1];

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
        auto* output = static_cast<SyclTensor*>(output_handle->plugin_data);

        auto* stream_handle = ctx.get_stream(&status);
        if (stream_handle == nullptr) {
            ctx.fail(&status);
            return;
        }
        auto* stream = static_cast<SyclStream*>(stream_handle->plugin_data);

        run_woq_matmul(
            *stream,
            m,
            k,
            n,
            attrs.group_size,
            raw_data(*activation),
            raw_data(*weight),
            raw_data(*scale),
            raw_data(*zero_point),
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

    static void run_woq_matmul(
        SyclStream& stream,
        int64_t m,
        int64_t k,
        int64_t n,
        int64_t group_size,
        void* activation_data,
        void* packed_weight_data,
        void* scale_data,
        void* zero_point_data,
        void* output_data
    )
    {
        sycl::queue& queue = stream.get_native_queue();
        dnnl::engine& engine =
            EngineCache::instance().get_engine(queue.get_device(), queue.get_context());
        dnnl::stream& dnnl_stream = EngineCache::instance().get_stream(engine, queue);

        auto activation_type = dnnl::memory::data_type::f32;
        dnnl::memory::desc activation_md{{m, k}, activation_type, dnnl::memory::format_tag::ab};
        dnnl::memory::desc dst_md{{m, n}, activation_type, dnnl::memory::format_tag::ab};

        // The packed weight is reinterpreted as u4 directly at its real address — same trick
        // WoQMatmul.cpp uses (m2_u4_m wraps m2_usr_m's data_handle), rather than an extra copy.
        dnnl::memory::desc weight_md{
            {k, n},
            dnnl::memory::data_type::u4,
            dnnl::memory::format_tag::ab
        };
        dnnl::memory weight_memory{weight_md, engine, packed_weight_data};

        dnnl::primitive_attr attributes;
        attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);
        // Per-group scale/zero-point along both the K (mask bit 0) and N (mask bit 1) axes —
        // matches WoQMatmul.cpp's set_scales/set_zero_points mask (1<<0)+(1<<1).
        attributes
            .set_scales(DNNL_ARG_WEIGHTS, (1 << 0) + (1 << 1), {group_size, 1}, activation_type);
        attributes.set_zero_points(
            DNNL_ARG_WEIGHTS,
            (1 << 0) + (1 << 1),
            {group_size, 1},
            dnnl::memory::data_type::s8
        );

        dnnl::matmul::primitive_desc
            primitive_desc{engine, activation_md, weight_md, dst_md, attributes};
        dnnl::matmul matmul{primitive_desc};

        const int64_t num_groups = k / group_size;
        dnnl::memory::desc scale_md{{num_groups, n}, activation_type, dnnl::memory::format_tag::ab};
        dnnl::memory::desc zero_point_md{
            {num_groups, n},
            dnnl::memory::data_type::s8,
            dnnl::memory::format_tag::ab
        };

        std::unordered_map<int, dnnl::memory> arguments;
        arguments.emplace(DNNL_ARG_SRC, dnnl::memory{activation_md, engine, activation_data});
        arguments.emplace(DNNL_ARG_WEIGHTS, weight_memory);
        arguments.emplace(DNNL_ARG_DST, dnnl::memory{dst_md, engine, output_data});
        arguments.emplace(
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_WEIGHTS,
            dnnl::memory{scale_md, engine, scale_data}
        );
        arguments.emplace(
            DNNL_ARG_ATTR_ZERO_POINTS | DNNL_ARG_WEIGHTS,
            dnnl::memory{zero_point_md, engine, zero_point_data}
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
