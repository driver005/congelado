module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

#include <oneapi/dnnl/dnnl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:attention_sdpa_math;

import std;
import aten_xpu_intern;
import :context;
import :onednn_memory_layout;
import :onednn_post_op_attributes;
import :onednn_matmul_primitive;
import :onednn_primitive_executor;

export namespace aten_xpu {

class SyclSdpaMath
{
public:
    explicit SyclSdpaMath(SyclKernelContext& context, sycl::queue& queue) :
        m_context{context},
        m_queue{queue}
    {
    }

    ~SyclSdpaMath()
    {
        for (auto& view: m_views) {
            view.get().destroy();
        }
    }

    SyclSdpaMath(const SyclSdpaMath&) = delete;
    SyclSdpaMath& operator=(const SyclSdpaMath&) = delete;
    SyclSdpaMath(SyclSdpaMath&&) = delete;
    SyclSdpaMath& operator=(SyclSdpaMath&&) = delete;

    std::optional<std::reference_wrapper<SyclTensor>> forward(
        const SyclTensor& query,
        const SyclTensor& key,
        const SyclTensor& value,
        std::optional<std::reference_wrapper<const SyclTensor>> mask,
        SyclTensor& output,
        float scale,
        bool is_causal
    )
    {
        read_shape(query, key);
        auto& query_view = view3(query, m_groups * m_query_length, m_head_dim);
        auto& key_view = view3(key, m_key_length, m_head_dim);
        auto& value_view = view3(value, m_key_length, value.getDims()[3]);
        auto& output_view = view3(output, m_groups * m_query_length, value.getDims()[3]);

        const std::array<int64_t, 3> score_dims{
            m_batch_heads,
            m_groups * m_query_length,
            m_key_length
        };
        auto scores = m_context.allocateTemp(TF_FLOAT, score_dims);
        if (!scores) {
            return std::nullopt;
        }

        SyclPostOpAttributes score_ops;
        score_ops.addEltwise(1.0F, scale, 0.0F, dnnl::algorithm::eltwise_linear);
        if (mask) {
            if (m_groups != 1) {
                throw std::invalid_argument{
                    "explicit attention masks need equal query and key heads"
                };
            }
            score_ops.addBinary(dnnl::algorithm::binary_add, mask_view(mask->get()), true);
        } else if (is_causal) {
            score_ops.addBinary(dnnl::algorithm::binary_add, causal_mask(), true);
        }
        SyclMatmulPrimitive::run(
            m_queue,
            query_view,
            key_view,
            std::nullopt,
            scores->get(),
            score_ops,
            true
        );
        softmax_forward(scores->get());

        auto& probabilities = probabilities_as(scores->get(), query.getDtype());
        SyclMatmulPrimitive::run(
            m_queue,
            probabilities,
            value_view,
            std::nullopt,
            output_view,
            SyclPostOpAttributes{}
        );
        return scores;
    }

    void backward(
        const SyclTensor& gradient_output,
        const SyclTensor& query,
        const SyclTensor& key,
        const SyclTensor& value,
        const SyclTensor& probabilities,
        SyclTensor& gradient_query,
        SyclTensor& gradient_key,
        SyclTensor& gradient_value,
        float scale
    )
    {
        read_shape(query, key);
        if (m_groups != 1) {
            throw std::invalid_argument{"attention backward needs equal query and key heads"};
        }
        const auto value_dim = value.getDims()[3];
        auto& gradient_output_view = view3(gradient_output, m_query_length, value_dim);
        auto& query_view = view3(query, m_query_length, m_head_dim);
        auto& key_view = view3(key, m_key_length, m_head_dim);
        auto& value_view = view3(value, m_key_length, value_dim);
        auto& gradient_query_view = view3(gradient_query, m_query_length, m_head_dim);
        auto& gradient_key_view = view3(gradient_key, m_key_length, m_head_dim);
        auto& gradient_value_view = view3(gradient_value, m_key_length, value_dim);

        auto& probabilities_typed =
            probabilities_as(const_cast<SyclTensor&>(probabilities), query.getDtype());
        SyclMatmulPrimitive::run(
            m_queue,
            transpose(probabilities_typed),
            gradient_output_view,
            std::nullopt,
            gradient_value_view,
            SyclPostOpAttributes{}
        );

        const std::array<int64_t, 3> score_dims{m_batch_heads, m_query_length, m_key_length};
        auto gradient_probabilities = m_context.allocateTemp(TF_FLOAT, score_dims);
        auto gradient_scores = m_context.allocateTemp(TF_FLOAT, score_dims);
        if (!gradient_probabilities || !gradient_scores) {
            return;
        }
        SyclMatmulPrimitive::run(
            m_queue,
            gradient_output_view,
            value_view,
            std::nullopt,
            gradient_probabilities->get(),
            SyclPostOpAttributes{},
            true
        );
        softmax_backward(probabilities, gradient_probabilities->get(), gradient_scores->get());

        auto& gradient_scores_typed = probabilities_as(gradient_scores->get(), query.getDtype());
        SyclPostOpAttributes scale_ops;
        scale_ops.addEltwise(1.0F, scale, 0.0F, dnnl::algorithm::eltwise_linear);
        SyclMatmulPrimitive::run(
            m_queue,
            gradient_scores_typed,
            key_view,
            std::nullopt,
            gradient_query_view,
            scale_ops
        );
        SyclMatmulPrimitive::run(
            m_queue,
            transpose(gradient_scores_typed),
            query_view,
            std::nullopt,
            gradient_key_view,
            scale_ops
        );
    }

private:
    void read_shape(const SyclTensor& query, const SyclTensor& key)
    {
        const auto& query_dims = query.getDims();
        const auto& key_dims = key.getDims();
        m_groups = query_dims[1] / key_dims[1];
        m_batch_heads = key_dims[0] * key_dims[1];
        m_query_length = query_dims[2];
        m_key_length = key_dims[2];
        m_head_dim = query_dims[3];
    }

    SyclTensor& remember(::TF_Tensor* handle)
    {
        auto& view = SyclHandle::resolve_raw<SyclTensor>(handle);
        m_views.emplace_back(view);
        return view;
    }

    SyclTensor& view3(const SyclTensor& tensor, int64_t rows, int64_t columns)
    {
        const std::array<int64_t, 3> dims{m_batch_heads, rows, columns};
        const std::array<int64_t, 3> strides{rows * columns, columns, 1};
        ::TF_Tensor* handle = nullptr;
        const_cast<SyclTensor&>(tensor).tensor_view(
            dims.data(),
            3,
            strides.data(),
            tensor.getStorageOffset(),
            &handle,
            m_context.getStatus()
        );
        return remember(handle);
    }

    SyclTensor& transpose(const SyclTensor& tensor)
    {
        const auto& dims = tensor.getDims();
        const auto& strides = tensor.getStrides();
        const std::array<int64_t, 3> swapped_dims{dims[0], dims[2], dims[1]};
        const std::array<int64_t, 3> swapped_strides{strides[0], strides[2], strides[1]};
        ::TF_Tensor* handle = nullptr;
        const_cast<SyclTensor&>(tensor).tensor_view(
            swapped_dims.data(),
            3,
            swapped_strides.data(),
            tensor.getStorageOffset(),
            &handle,
            m_context.getStatus()
        );
        return remember(handle);
    }

    SyclTensor& mask_view(const SyclTensor& mask)
    {
        const auto& dims = mask.getDims();
        const auto& strides = mask.getStrides();
        const auto rank = dims.size();
        const int64_t batch_stride = rank >= 3 ? strides[rank - 3] : 0;
        const int64_t batch_extent =
            rank >= 3
                ? std::accumulate(dims.begin(), dims.end() - 2, int64_t{1}, std::multiplies<>{})
                : 1;
        const std::array<int64_t, 3> view_dims{batch_extent, dims[rank - 2], dims[rank - 1]};
        const std::array<int64_t, 3> view_strides{
            batch_stride,
            strides[rank - 2],
            strides[rank - 1]
        };
        ::TF_Tensor* handle = nullptr;
        const_cast<SyclTensor&>(mask).tensor_view(
            view_dims.data(),
            3,
            view_strides.data(),
            mask.getStorageOffset(),
            &handle,
            m_context.getStatus()
        );
        return remember(handle);
    }

    SyclTensor& causal_mask()
    {
        const std::array<int64_t, 3> dims{1, m_groups * m_query_length, m_key_length};
        auto mask = m_context.allocateTemp(TF_FLOAT, dims);
        if (!mask) {
            throw std::runtime_error{"failed to allocate causal mask"};
        }
        auto* data = static_cast<float*>(mask->get().getData());
        const auto query_length = m_query_length;
        const auto key_length = m_key_length;
        const auto offset = key_length - query_length;
        m_queue.parallel_for(
            sycl::range<2>{static_cast<std::size_t>(dims[1]), static_cast<std::size_t>(key_length)},
            [=](sycl::id<2> index)
            {
                const auto row = static_cast<int64_t>(index[0]) % query_length;
                const auto column = static_cast<int64_t>(index[1]);
                data[index[0] * key_length + index[1]] =
                    column > row + offset ? -std::numeric_limits<float>::infinity() : 0.0F;
            }
        );
        return mask->get();
    }

    SyclTensor& probabilities_as(SyclTensor& scores, TFDataTypeEnum dtype)
    {
        if (dtype == TF_FLOAT) {
            return scores;
        }
        auto converted = m_context.allocateTemp(dtype, scores.getDims());
        if (!converted) {
            throw std::runtime_error{"failed to allocate probabilities"};
        }
        SyclPrimitiveExecutor executor{m_queue};
        const auto source_desc = SyclOnednnLayout::desc(scores);
        const auto destination_desc = SyclOnednnLayout::desc(converted->get());
        const dnnl::reorder::primitive_desc primitive_desc{
            executor.getEngine(),
            source_desc,
            executor.getEngine(),
            destination_desc
        };
        executor.addArgument(DNNL_ARG_FROM, source_desc, scores.getData());
        executor.addArgument(DNNL_ARG_TO, destination_desc, converted->get().getData());
        executor.execute(dnnl::reorder{primitive_desc}, primitive_desc);
        return converted->get();
    }

    void softmax_forward(SyclTensor& scores)
    {
        SyclPrimitiveExecutor executor{m_queue};
        const auto desc = SyclOnednnLayout::desc(scores);
        const dnnl::softmax_forward::primitive_desc primitive_desc{
            executor.getEngine(),
            dnnl::prop_kind::forward_training,
            dnnl::algorithm::softmax_accurate,
            desc,
            desc,
            2,
            SyclPrimitiveExecutor::user_scratchpad_attributes()
        };
        executor.addArgument(DNNL_ARG_SRC, desc, scores.getData());
        executor.addArgument(DNNL_ARG_DST, desc, scores.getData());
        executor.execute(dnnl::softmax_forward{primitive_desc}, primitive_desc);
    }

    void softmax_backward(
        const SyclTensor& probabilities,
        const SyclTensor& gradient_probabilities,
        SyclTensor& gradient_scores
    )
    {
        SyclPrimitiveExecutor executor{m_queue};
        const auto desc = SyclOnednnLayout::desc(probabilities);
        const dnnl::softmax_forward::primitive_desc hint{
            executor.getEngine(),
            dnnl::prop_kind::forward_training,
            dnnl::algorithm::softmax_accurate,
            desc,
            desc,
            2
        };
        const dnnl::softmax_backward::primitive_desc primitive_desc{
            executor.getEngine(),
            dnnl::algorithm::softmax_accurate,
            desc,
            desc,
            desc,
            2,
            hint,
            SyclPrimitiveExecutor::user_scratchpad_attributes()
        };
        executor.addArgument(DNNL_ARG_DST, desc, probabilities.getData());
        executor.addArgument(DNNL_ARG_DIFF_DST, desc, gradient_probabilities.getData());
        executor.addArgument(DNNL_ARG_DIFF_SRC, desc, gradient_scores.getData());
        executor.execute(dnnl::softmax_backward{primitive_desc}, primitive_desc);
    }

    SyclKernelContext& m_context;
    sycl::queue& m_queue;
    std::vector<std::reference_wrapper<SyclTensor>> m_views;
    int64_t m_groups{1};
    int64_t m_batch_heads{0};
    int64_t m_query_length{0};
    int64_t m_key_length{0};
    int64_t m_head_dim{0};
};

} // namespace aten_xpu
