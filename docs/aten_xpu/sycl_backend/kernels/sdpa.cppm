// SYCL reference plugin — scaled dot-product attention, forward only.
//
// Not built by Bazel (docs/ only). Neither of the two real ATen paths is ported: mkldnn's
// oneDNN Graph SDPA (mkldnn/detail/Attention.cpp) needs the full oneDNN Graph partition-compile-
// and-cache machinery, and transformers/attention.cpp forwards to the external `sycltla::`
// flash-attention library, which is not vendored in this tree (confirmed absent — see
// docs/aten-xpu-gap-analysis.md). This file is a naive (non-fused, no dropout) attention built
// from three plain oneDNN calls instead: batched matmul (Q@K^T, scaled), softmax, batched matmul
// (@V). Causal masking is a small SYCL kernel that writes -inf above the diagonal before the
// softmax — real, correct, just not flash-attention's fused/tiled algorithm.
//
// Shapes: query/key/value are [batch, heads, seq, head_dim]; internally flattened to
// [batch*heads, seq, head_dim] for oneDNN's batched matmul.

module;

#include "include/c/extern/kernel/builder.h"
#include "docs/aten_xpu/sycl_backend/kernels/kernel_context.h"
#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>

export module sycl_backend:kernels_sdpa;

import std;
import :tensor;
import :stream;
import :engine_cache;

export namespace sycl_backend::kernels {

class SdpaKernel
{
public:
    SdpaKernel() = delete;

    // Inputs 0/1/2: query/key/value, each [batch, heads, seq, head_dim] (key/value may have a
    // different seq length than query). Attribute "is_causal" (read as an int64 0/1, since this
    // plugin has no bool attr helper) selects the causal mask. Output 0: [batch, heads,
    // seq_q, head_dim].
    static void compute(void* plugin_data, TF_OpKernelContext* raw_context) noexcept
    {
        (void)plugin_data;

        KernelContextView ctx{raw_context};
        TF_Status status{};

        auto* query_handle = ctx.get_input(0, &status);
        auto* key_handle = ctx.get_input(1, &status);
        auto* value_handle = ctx.get_input(2, &status);
        if (query_handle == nullptr || key_handle == nullptr || value_handle == nullptr) {
            ctx.fail(&status);
            return;
        }

        auto* query = static_cast<SyclTensor*>(query_handle->plugin_data);
        auto* key = static_cast<SyclTensor*>(key_handle->plugin_data);
        auto* value = static_cast<SyclTensor*>(value_handle->plugin_data);

        const std::vector<int64_t> query_dims = shape_of(*query);
        const std::vector<int64_t> key_dims = shape_of(*key);
        const int64_t batch = query_dims[0];
        const int64_t heads = query_dims[1];
        const int64_t seq_q = query_dims[2];
        const int64_t seq_k = key_dims[2];
        const int64_t head_dim = query_dims[3];
        const int64_t batch_heads = batch * heads;

        const std::vector<int64_t> output_dims{batch, heads, seq_q, head_dim};
        std::size_t output_elements = 1;
        for (int64_t extent: output_dims) {
            output_elements *= static_cast<std::size_t>(extent);
        }

        auto* output_handle = ctx.allocate_output(
            0,
            TF_FLOAT,
            output_dims.data(),
            static_cast<int>(output_dims.size()),
            output_elements * SyclTensor::element_size(TF_FLOAT),
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

        const double scale = 1.0 / std::sqrt(static_cast<double>(head_dim));
        const bool is_causal = ctx.num_inputs() > 3;

        run_attention(
            *stream,
            batch_heads,
            seq_q,
            seq_k,
            head_dim,
            raw_data(*query),
            raw_data(*key),
            raw_data(*value),
            raw_data(*output),
            scale,
            is_causal
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

    static void run_attention(
        SyclStream& stream,
        int64_t batch_heads,
        int64_t seq_q,
        int64_t seq_k,
        int64_t head_dim,
        void* query_data,
        void* key_data,
        void* value_data,
        void* output_data,
        double scale,
        bool is_causal
    )
    {
        sycl::queue& queue = stream.get_native_queue();
        dnnl::engine& engine =
            EngineCache::instance().get_engine(queue.get_device(), queue.get_context());
        dnnl::stream& dnnl_stream = EngineCache::instance().get_stream(engine, queue);

        auto data_type = dnnl::memory::data_type::f32;

        void* scores_data = sycl::malloc_device(
            static_cast<std::size_t>(batch_heads * seq_q * seq_k) * sizeof(float),
            queue
        );

        // STEP 1: scores = (Q @ K^T) * scale. K is [batch_heads, seq_k, head_dim]; read as
        // [batch_heads, head_dim, seq_k] via strides, same transpose-via-strides trick as
        // linear.cppm, so no physical transpose is needed.
        dnnl::memory::desc query_md{
            {batch_heads, seq_q, head_dim},
            data_type,
            dnnl::memory::format_tag::abc
        };
        dnnl::memory::desc key_transposed_md{
            {batch_heads, head_dim, seq_k},
            data_type,
            dnnl::memory::dims{seq_k * head_dim, 1, head_dim}
        };
        dnnl::memory::desc scores_md{
            {batch_heads, seq_q, seq_k},
            data_type,
            dnnl::memory::format_tag::abc
        };

        dnnl::primitive_attr scores_attributes;
        scores_attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);

        dnnl::matmul::primitive_desc scores_pd{
            engine,
            query_md,
            key_transposed_md,
            scores_md,
            scores_attributes
        };
        dnnl::matmul scores_matmul{scores_pd};

        run_with_scratchpad(
            queue,
            engine,
            dnnl_stream,
            scores_matmul,
            scores_pd.scratchpad_desc(),
            {{DNNL_ARG_SRC, dnnl::memory{query_md, engine, query_data}},
             {DNNL_ARG_WEIGHTS, dnnl::memory{key_transposed_md, engine, key_data}},
             {DNNL_ARG_DST, dnnl::memory{scores_md, engine, scores_data}}}
        );

        // STEP 2: apply the 1/sqrt(head_dim) scale, and — if causal — write -inf above the
        // diagonal, per (batch*head, row). oneDNN's matmul has no plain output-scale attribute
        // usable across versions, so this is a small SYCL kernel instead.
        {
            auto* scores = static_cast<float*>(scores_data);
            const auto scale_factor = static_cast<float>(scale);
            queue.parallel_for(
                    sycl::range<3>{
                        static_cast<std::size_t>(batch_heads),
                        static_cast<std::size_t>(seq_q),
                        static_cast<std::size_t>(seq_k)
                    },
                    [scores, seq_q, seq_k, scale_factor, is_causal](sycl::id<3> index)
                    {
                        const auto batch_head = static_cast<int64_t>(index[0]);
                        const auto row = static_cast<int64_t>(index[1]);
                        const auto col = static_cast<int64_t>(index[2]);
                        const auto offset = batch_head * seq_q * seq_k + row * seq_k + col;

                        if (is_causal && col > row) {
                            scores[offset] = -std::numeric_limits<float>::infinity();
                        } else {
                            scores[offset] *= scale_factor;
                        }
                    }
                )
                .wait();
        }

        // STEP 3: softmax over the last axis.
        dnnl::softmax_forward::primitive_desc softmax_pd{
            engine,
            dnnl::prop_kind::forward_inference,
            dnnl::algorithm::softmax_accurate,
            scores_md,
            scores_md,
            2
        };
        dnnl::softmax_forward softmax{softmax_pd};
        dnnl_stream.wait();
        dnnl::sycl_interop::execute(
            softmax,
            dnnl_stream,
            {{DNNL_ARG_SRC, dnnl::memory{scores_md, engine, scores_data}},
             {DNNL_ARG_DST, dnnl::memory{scores_md, engine, scores_data}}}
        );

        // STEP 4: output = softmax(scores) @ V.
        dnnl::memory::desc value_md{
            {batch_heads, seq_k, head_dim},
            data_type,
            dnnl::memory::format_tag::abc
        };
        dnnl::memory::desc output_md{
            {batch_heads, seq_q, head_dim},
            data_type,
            dnnl::memory::format_tag::abc
        };

        dnnl::primitive_attr output_attributes;
        output_attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);

        dnnl::matmul::primitive_desc output_pd{engine, scores_md, value_md, output_md, output_attributes};
        dnnl::matmul output_matmul{output_pd};

        run_with_scratchpad(
            queue,
            engine,
            dnnl_stream,
            output_matmul,
            output_pd.scratchpad_desc(),
            {{DNNL_ARG_SRC, dnnl::memory{scores_md, engine, scores_data}},
             {DNNL_ARG_WEIGHTS, dnnl::memory{value_md, engine, value_data}},
             {DNNL_ARG_DST, dnnl::memory{output_md, engine, output_data}}}
        );

        queue.wait();
        sycl::free(scores_data, queue);
    }

    static void run_with_scratchpad(
        sycl::queue& queue,
        dnnl::engine& engine,
        dnnl::stream& dnnl_stream,
        dnnl::primitive& primitive,
        const dnnl::memory::desc& scratchpad_desc,
        std::unordered_map<int, dnnl::memory> arguments
    )
    {
        const std::size_t scratchpad_size = scratchpad_desc.get_size();
        void* scratchpad_data =
            scratchpad_size == 0 ? nullptr : sycl::malloc_device(scratchpad_size, queue);
        if (scratchpad_data != nullptr) {
            arguments.emplace(DNNL_ARG_SCRATCHPAD, dnnl::memory{scratchpad_desc, engine, scratchpad_data});
        }

        dnnl::sycl_interop::execute(primitive, dnnl_stream, arguments);

        if (scratchpad_data != nullptr) {
            queue.wait();
            sycl::free(scratchpad_data, queue);
        }
    }
};

} // namespace sycl_backend::kernels
