// SYCL reference plugin — per-tensor int8 quantized 2D convolution via oneDNN.
//
// Not built by Bazel (docs/ only). Replaces mkldnn/detail/QConv.cpp's quantized_convolution for
// the per-tensor-quantization, ungrouped, forward-only case (mask=0 for src/weight/dst) — no
// per-channel weight scale, no binary/unary post-ops, no accum tensor.

module;

#include "include/c/extern/kernel/builder.h"
#include "docs/aten_xpu/sycl_backend/kernels/kernel_context.h"
#include "docs/aten_xpu/sycl_backend/kernels/kernel_construction_view.h"
#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>

export module sycl_backend:kernels_int8_conv;

import std;
import :tensor;
import :stream;
import :engine_cache;

export namespace sycl_backend::kernels {

class Int8ConvAttrs
{
public:
    int64_t stride{1};
    int64_t padding{0};
    int64_t dilation{1};
};

class Int8ConvKernel
{
public:
    Int8ConvKernel() = delete;

    static void create(TF_OpKernelConstruction* raw_construction, void** out_plugin_data) noexcept
    {
        KernelConstructionView construction{raw_construction};
        TF_Status status{};

        auto* attrs = new Int8ConvAttrs{};
        attrs->stride = construction.get_attr_int64("stride", 1, &status);
        attrs->padding = construction.get_attr_int64("padding", 0, &status);
        attrs->dilation = construction.get_attr_int64("dilation", 1, &status);

        *out_plugin_data = attrs;
    }

    static void destroy(void* plugin_data) noexcept
    {
        delete static_cast<Int8ConvAttrs*>(plugin_data);
    }

    // Inputs 0/1: source/weight (s8, NCHW / OIHW). Inputs 2/3: source scale/zero point (f32/s32
    // scalar). Input 4: weight scale (f32 scalar). Inputs 5/6: output scale/zero point. Output
    // 0: s8, NCHW.
    static void compute(void* plugin_data, TF_OpKernelContext* raw_context) noexcept
    {
        const auto& attrs = *static_cast<Int8ConvAttrs*>(plugin_data);
        KernelContextView ctx{raw_context};
        TF_Status status{};

        auto* source_handle = ctx.get_input(0, &status);
        auto* weight_handle = ctx.get_input(1, &status);
        auto* source_scale_handle = ctx.get_input(2, &status);
        auto* source_zp_handle = ctx.get_input(3, &status);
        auto* weight_scale_handle = ctx.get_input(4, &status);
        auto* output_scale_handle = ctx.get_input(5, &status);
        auto* output_zp_handle = ctx.get_input(6, &status);
        if (source_handle == nullptr || weight_handle == nullptr ||
            source_scale_handle == nullptr || source_zp_handle == nullptr ||
            weight_scale_handle == nullptr || output_scale_handle == nullptr ||
            output_zp_handle == nullptr)
        {
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

        std::size_t element_count = 1;
        for (int64_t extent: dst_dims) {
            element_count *= static_cast<std::size_t>(extent);
        }

        auto* output_handle = ctx.allocate_output(
            0,
            TF_QINT8,
            dst_dims.data(),
            static_cast<int>(dst_dims.size()),
            element_count,
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

        run_int8_convolution(
            *stream,
            source_dims,
            weight_dims,
            dst_dims,
            raw_data(*source),
            raw_data(*weight),
            raw_data(*dst),
            raw_data(*static_cast<SyclTensor*>(source_scale_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(source_zp_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(weight_scale_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(output_scale_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(output_zp_handle->plugin_data)),
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

    static void run_int8_convolution(
        SyclStream& stream,
        const std::vector<int64_t>& source_dims,
        const std::vector<int64_t>& weight_dims,
        const std::vector<int64_t>& dst_dims,
        void* source_data,
        void* weight_data,
        void* dst_data,
        void* source_scale_data,
        void* source_zp_data,
        void* weight_scale_data,
        void* output_scale_data,
        void* output_zp_data,
        const Int8ConvAttrs& attrs
    )
    {
        sycl::queue& queue = stream.get_native_queue();
        dnnl::engine& engine =
            EngineCache::instance().get_engine(queue.get_device(), queue.get_context());
        dnnl::stream& dnnl_stream = EngineCache::instance().get_stream(engine, queue);

        auto quant_type = dnnl::memory::data_type::s8;
        dnnl::memory::desc source_md{source_dims, quant_type, dnnl::memory::format_tag::nchw};
        dnnl::memory::desc weight_md{weight_dims, quant_type, dnnl::memory::format_tag::oihw};
        dnnl::memory::desc dst_md{dst_dims, quant_type, dnnl::memory::format_tag::nchw};
        dnnl::memory::desc scalar_md{{1}, dnnl::memory::data_type::f32, dnnl::memory::format_tag::x};
        dnnl::memory::desc zp_md{{1}, dnnl::memory::data_type::s32, dnnl::memory::format_tag::x};

        dnnl::primitive_attr attributes;
        attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);
        attributes.set_scales_mask(DNNL_ARG_SRC, 0);
        attributes.set_zero_points_mask(DNNL_ARG_SRC, 0);
        attributes.set_scales_mask(DNNL_ARG_WEIGHTS, 0);
        attributes.set_scales_mask(DNNL_ARG_DST, 0);
        attributes.set_zero_points_mask(DNNL_ARG_DST, 0);

        auto primitive_desc = dnnl::convolution_forward::primitive_desc{
            engine,
            dnnl::prop_kind::forward,
            dnnl::algorithm::convolution_direct,
            source_md,
            weight_md,
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
        arguments.emplace(
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_SRC,
            dnnl::memory{scalar_md, engine, source_scale_data}
        );
        arguments.emplace(
            DNNL_ARG_ATTR_ZERO_POINTS | DNNL_ARG_SRC,
            dnnl::memory{zp_md, engine, source_zp_data}
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

        dnnl::sycl_interop::execute(convolution, dnnl_stream, arguments);

        if (scratchpad_data != nullptr) {
            queue.wait();
            sycl::free(scratchpad_data, queue);
        }
    }
};

} // namespace sycl_backend::kernels
