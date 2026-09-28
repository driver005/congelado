// SYCL reference plugin — scaled dot-product attention backward, matching sdpa.cppm's naive
// (non-fused, no dropout, no attn_mask) forward algorithm.
//
// Not built by Bazel (docs/ only). Replaces transformers/attention_backward.cpp, which forwards
// to the external (not vendored) `sycltla::flash_attention_backward`. This recomputes the
// forward's scores/softmax internally rather than requiring sdpa.cppm to save them (a real
// memory-efficient backward would take the saved softmax output — or an equivalent
// logsumexp — as an input instead of recomputing it):
//   P = softmax(scale * Q @ K^T [+ causal mask])   (recomputed)
//   dV = P^T @ dOut
//   dP = dOut @ V^T
//   dScores = P * (dP - rowsum(dP * P))            (softmax backward, elementwise)
//   dQ = scale * dScores @ K
//   dK = scale * dScores^T @ Q

module;

#include "include/c/extern/kernel/builder.h"
#include "docs/aten_xpu/sycl_backend/kernels/kernel_context.h"
#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_sycl.hpp>

export module sycl_backend:kernels_sdpa_backward;

import std;
import :tensor;
import :stream;
import :engine_cache;

export namespace sycl_backend::kernels {

class SdpaBackwardKernel
{
public:
    SdpaBackwardKernel() = delete;

    // Inputs 0/1/2: query/key/value, each [batch, heads, seq, head_dim] (as in sdpa.cppm).
    // Input 3: dOutput, same shape as the forward output. Outputs 0/1/2: dQuery, dKey, dValue.
    static void compute(void* plugin_data, TF_OpKernelContext* raw_context) noexcept
    {
        (void)plugin_data;

        KernelContextView ctx{raw_context};
        TF_Status status{};

        auto* query_handle = ctx.get_input(0, &status);
        auto* key_handle = ctx.get_input(1, &status);
        auto* value_handle = ctx.get_input(2, &status);
        auto* grad_output_handle = ctx.get_input(3, &status);
        if (query_handle == nullptr || key_handle == nullptr || value_handle == nullptr ||
            grad_output_handle == nullptr)
        {
            ctx.fail(&status);
            return;
        }

        auto* query = static_cast<SyclTensor*>(query_handle->plugin_data);
        auto* key = static_cast<SyclTensor*>(key_handle->plugin_data);
        auto* value = static_cast<SyclTensor*>(value_handle->plugin_data);
        auto* grad_output = static_cast<SyclTensor*>(grad_output_handle->plugin_data);

        const std::vector<int64_t> query_dims = shape_of(*query);
        const std::vector<int64_t> key_dims = shape_of(*key);
        const int64_t batch = query_dims[0];
        const int64_t heads = query_dims[1];
        const int64_t seq_q = query_dims[2];
        const int64_t seq_k = key_dims[2];
        const int64_t head_dim = query_dims[3];
        const int64_t batch_heads = batch * heads;

        auto* grad_query_handle = ctx.allocate_output(
            0,
            TF_FLOAT,
            query_dims.data(),
            static_cast<int>(query_dims.size()),
            static_cast<std::size_t>(batch_heads * seq_q * head_dim) *
                SyclTensor::element_size(TF_FLOAT),
            &status
        );
        auto* grad_key_handle = ctx.allocate_output(
            1,
            TF_FLOAT,
            key_dims.data(),
            static_cast<int>(key_dims.size()),
            static_cast<std::size_t>(batch_heads * seq_k * head_dim) *
                SyclTensor::element_size(TF_FLOAT),
            &status
        );
        auto* grad_value_handle = ctx.allocate_output(
            2,
            TF_FLOAT,
            key_dims.data(),
            static_cast<int>(key_dims.size()),
            static_cast<std::size_t>(batch_heads * seq_k * head_dim) *
                SyclTensor::element_size(TF_FLOAT),
            &status
        );
        if (grad_query_handle == nullptr || grad_key_handle == nullptr ||
            grad_value_handle == nullptr)
        {
            ctx.fail(&status);
            return;
        }

        auto* stream_handle = ctx.get_stream(&status);
        if (stream_handle == nullptr) {
            ctx.fail(&status);
            return;
        }
        auto* stream = static_cast<SyclStream*>(stream_handle->plugin_data);

        const double scale = 1.0 / std::sqrt(static_cast<double>(head_dim));
        const bool is_causal = ctx.num_inputs() > 4;

        run_backward(
            *stream,
            batch_heads,
            seq_q,
            seq_k,
            head_dim,
            raw_data(*query),
            raw_data(*key),
            raw_data(*value),
            raw_data(*grad_output),
            raw_data(*static_cast<SyclTensor*>(grad_query_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(grad_key_handle->plugin_data)),
            raw_data(*static_cast<SyclTensor*>(grad_value_handle->plugin_data)),
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

    static dnnl::memory matmul_into(
        dnnl::engine& engine,
        dnnl::stream& dnnl_stream,
        sycl::queue& queue,
        const dnnl::memory::desc& lhs_md,
        void* lhs_data,
        const dnnl::memory::desc& rhs_md,
        void* rhs_data,
        const dnnl::memory::desc& dst_md,
        void* dst_data
    )
    {
        dnnl::primitive_attr attributes;
        attributes.set_scratchpad_mode(dnnl::scratchpad_mode::user);
        dnnl::matmul::primitive_desc primitive_desc{engine, lhs_md, rhs_md, dst_md, attributes};
        dnnl::matmul matmul{primitive_desc};

        std::unordered_map<int, dnnl::memory> arguments;
        arguments.emplace(DNNL_ARG_SRC, dnnl::memory{lhs_md, engine, lhs_data});
        arguments.emplace(DNNL_ARG_WEIGHTS, dnnl::memory{rhs_md, engine, rhs_data});
        dnnl::memory dst{dst_md, engine, dst_data};
        arguments.emplace(DNNL_ARG_DST, dst);

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

        return dst;
    }

    static void run_backward(
        SyclStream& stream,
        int64_t batch_heads,
        int64_t seq_q,
        int64_t seq_k,
        int64_t head_dim,
        void* query_data,
        void* key_data,
        void* value_data,
        void* grad_output_data,
        void* grad_query_data,
        void* grad_key_data,
        void* grad_value_data,
        double scale,
        bool is_causal
    )
    {
        sycl::queue& queue = stream.get_native_queue();
        dnnl::engine& engine =
            EngineCache::instance().get_engine(queue.get_device(), queue.get_context());
        dnnl::stream& dnnl_stream = EngineCache::instance().get_stream(engine, queue);

        auto data_type = dnnl::memory::data_type::f32;
        const std::size_t scores_count = static_cast<std::size_t>(batch_heads * seq_q * seq_k);

        void* scores_data = sycl::malloc_device(scores_count * sizeof(float), queue);
        void* grad_scores_data = sycl::malloc_device(scores_count * sizeof(float), queue);

        dnnl::memory::desc query_md{{batch_heads, seq_q, head_dim}, data_type, dnnl::memory::format_tag::abc};
        dnnl::memory::desc key_transposed_md{
            {batch_heads, head_dim, seq_k},
            data_type,
            dnnl::memory::dims{seq_k * head_dim, 1, head_dim}
        };
        dnnl::memory::desc scores_md{{batch_heads, seq_q, seq_k}, data_type, dnnl::memory::format_tag::abc};

        // STEP 1: recompute P = softmax(scale * Q @ K^T [+ causal mask]).
        matmul_into(
            engine,
            dnnl_stream,
            queue,
            query_md,
            query_data,
            key_transposed_md,
            key_data,
            scores_md,
            scores_data
        );

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

        dnnl::softmax_forward::primitive_desc softmax_pd{
            engine,
            dnnl::prop_kind::forward_inference,
            dnnl::algorithm::softmax_accurate,
            scores_md,
            scores_md,
            2
        };
        dnnl::sycl_interop::execute(
            dnnl::softmax_forward{softmax_pd},
            dnnl_stream,
            {{DNNL_ARG_SRC, dnnl::memory{scores_md, engine, scores_data}},
             {DNNL_ARG_DST, dnnl::memory{scores_md, engine, scores_data}}}
        );
        queue.wait();

        // STEP 2: dV = P^T @ dOut. Read P as [batch_heads, seq_k, seq_q] via strides.
        dnnl::memory::desc value_grad_md{{batch_heads, seq_k, head_dim}, data_type, dnnl::memory::format_tag::abc};
        dnnl::memory::desc grad_output_md{{batch_heads, seq_q, head_dim}, data_type, dnnl::memory::format_tag::abc};
        dnnl::memory::desc scores_transposed_md{
            {batch_heads, seq_k, seq_q},
            data_type,
            dnnl::memory::dims{seq_q * seq_k, 1, seq_k}
        };
        matmul_into(
            engine,
            dnnl_stream,
            queue,
            scores_transposed_md,
            scores_data,
            grad_output_md,
            grad_output_data,
            value_grad_md,
            grad_value_data
        );

        // STEP 3: dP = dOut @ V^T.
        dnnl::memory::desc value_transposed_md{
            {batch_heads, head_dim, seq_k},
            data_type,
            dnnl::memory::dims{seq_k * head_dim, 1, head_dim}
        };
        matmul_into(
            engine,
            dnnl_stream,
            queue,
            grad_output_md,
            grad_output_data,
            value_transposed_md,
            value_data,
            scores_md,
            grad_scores_data
        );

        // STEP 4: dScores = P * (dP - rowsum(dP * P)), the softmax Jacobian-vector product.
        {
            auto* p = static_cast<float*>(scores_data);
            auto* dp = static_cast<float*>(grad_scores_data);
            queue.parallel_for(
                    sycl::range<2>{static_cast<std::size_t>(batch_heads), static_cast<std::size_t>(seq_q)},
                    [p, dp, seq_q, seq_k](sycl::id<2> index)
                    {
                        const auto batch_head = static_cast<int64_t>(index[0]);
                        const auto row = static_cast<int64_t>(index[1]);
                        const auto base = batch_head * seq_q * seq_k + row * seq_k;

                        float row_dot = 0.0F;
                        for (int64_t col = 0; col < seq_k; ++col) {
                            row_dot += dp[base + col] * p[base + col];
                        }
                        for (int64_t col = 0; col < seq_k; ++col) {
                            dp[base + col] = p[base + col] * (dp[base + col] - row_dot);
                        }
                    }
                )
                .wait();
        }

        // STEP 5: dQuery = scale * dScores @ K, dKey = scale * dScores^T @ Q.
        dnnl::memory::desc key_md{{batch_heads, seq_k, head_dim}, data_type, dnnl::memory::format_tag::abc};
        matmul_into(
            engine,
            dnnl_stream,
            queue,
            scores_md,
            grad_scores_data,
            key_md,
            key_data,
            query_md,
            grad_query_data
        );
        matmul_into(
            engine,
            dnnl_stream,
            queue,
            scores_transposed_md,
            grad_scores_data,
            query_md,
            query_data,
            key_md,
            grad_key_data
        );

        {
            auto* dq = static_cast<float*>(grad_query_data);
            auto* dk = static_cast<float*>(grad_key_data);
            const auto scale_factor = static_cast<float>(scale);
            queue.parallel_for(
                sycl::range<1>{static_cast<std::size_t>(batch_heads * seq_q * head_dim)},
                [dq, scale_factor](sycl::id<1> index) { dq[index] *= scale_factor; }
            );
            queue.parallel_for(
                sycl::range<1>{static_cast<std::size_t>(batch_heads * seq_k * head_dim)},
                [dk, scale_factor](sycl::id<1> index) { dk[index] *= scale_factor; }
            );
            queue.wait();
        }

        sycl::free(scores_data, queue);
        sycl::free(grad_scores_data, queue);
    }
};

} // namespace sycl_backend::kernels
