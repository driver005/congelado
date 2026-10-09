module;

#include <oneapi/dnnl/dnnl.hpp>
#include <oneapi/dnnl/dnnl_graph.hpp>
#include <oneapi/dnnl/dnnl_graph_sycl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:attention_onednn_sdpa;

import std;
import aten_xpu_intern;
import :onednn_engine_cache;
import :onednn_memory_layout;

export namespace aten_xpu {

class SyclOnednnSdpa
{
public:
    using Entry =
        std::pair<dnnl::graph::compiled_partition, std::vector<dnnl::graph::logical_tensor>>;

    SyclOnednnSdpa() = delete;

    static sycl::event
    run(sycl::queue& queue,
        const SyclTensor& query,
        const SyclTensor& key,
        const SyclTensor& value,
        std::optional<std::reference_wrapper<const SyclTensor>> mask,
        SyclTensor& output,
        float scale)
    {
        auto& engine = SyclEngineCache::getInstance().getEngine(queue);
        auto& stream = SyclEngineCache::getInstance().getStream(queue);
        const auto type = to_graph_type(query.getDtype());
        const auto key_text = cache_key(query, key, mask, type);

        auto found = s_cache.find(key_text);
        if (found == s_cache.end()) {
            found =
                s_cache.emplace(key_text, compile(engine, query, key, value, mask, output, type))
                    .first;
        }
        auto& [partition, inputs] = found->second;

        const float scale_value = scale;
        std::vector<dnnl::graph::tensor> input_tensors;
        input_tensors.reserve(inputs.size());
        input_tensors.emplace_back(inputs[0], engine, const_cast<void*>(query.getData()));
        input_tensors.emplace_back(inputs[1], engine, const_cast<void*>(key.getData()));
        input_tensors.emplace_back(inputs[2], engine, const_cast<float*>(&scale_value));
        std::size_t next = 3;
        if (mask) {
            input_tensors
                .emplace_back(inputs[next++], engine, const_cast<void*>(mask->get().getData()));
        }
        input_tensors.emplace_back(inputs[next], engine, const_cast<void*>(value.getData()));
        const dnnl::graph::tensor output_tensor{
            partition.query_logical_tensor(k_output_id),
            engine,
            output.getData()
        };
        return dnnl::graph::sycl_interop::execute(
            partition,
            stream,
            input_tensors,
            {output_tensor}
        );
    }

private:
    static constexpr std::size_t k_query_id = 0;
    static constexpr std::size_t k_key_id = 1;
    static constexpr std::size_t k_scores_id = 2;
    static constexpr std::size_t k_scale_id = 3;
    static constexpr std::size_t k_scaled_id = 4;
    static constexpr std::size_t k_mask_id = 5;
    static constexpr std::size_t k_masked_id = 6;
    static constexpr std::size_t k_probabilities_id = 7;
    static constexpr std::size_t k_value_id = 8;
    static constexpr std::size_t k_output_id = 9;

    static dnnl::graph::logical_tensor::data_type to_graph_type(TFDataTypeEnum dtype) noexcept
    {
        using data_type = dnnl::graph::logical_tensor::data_type;
        switch (dtype) {
            case TF_HALF:
                return data_type::f16;
            case TF_BFLOAT16:
                return data_type::bf16;
            default:
                return data_type::f32;
        }
    }

    static dnnl::graph::logical_tensor tensor_of(
        std::size_t identifier,
        const std::vector<int64_t>& dims,
        dnnl::graph::logical_tensor::data_type type
    )
    {
        return dnnl::graph::logical_tensor{
            identifier,
            type,
            dims,
            dnnl::graph::logical_tensor::layout_type::strided
        };
    }

    static Entry compile(
        dnnl::engine& engine,
        const SyclTensor& query,
        const SyclTensor& key,
        const SyclTensor& value,
        std::optional<std::reference_wrapper<const SyclTensor>> mask,
        const SyclTensor& output,
        dnnl::graph::logical_tensor::data_type type
    )
    {
        using graph_op = dnnl::graph::op;
        using float_type = dnnl::graph::logical_tensor::data_type;

        auto scores_dims = query.getDims();
        scores_dims[3] = key.getDims()[2];
        const auto query_tensor = tensor_of(k_query_id, query.getDims(), type);
        const auto key_tensor = tensor_of(k_key_id, key.getDims(), type);
        const auto scores_tensor = tensor_of(k_scores_id, scores_dims, float_type::f32);
        const auto scale_tensor = tensor_of(k_scale_id, {1}, float_type::f32);
        const auto scaled_tensor = tensor_of(k_scaled_id, scores_dims, float_type::f32);
        const auto probabilities_tensor = tensor_of(k_probabilities_id, scores_dims, type);
        const auto value_tensor = tensor_of(k_value_id, value.getDims(), type);
        const auto output_tensor = tensor_of(k_output_id, output.getDims(), type);

        graph_op first_matmul{
            0,
            graph_op::kind::MatMul,
            {query_tensor, key_tensor},
            {scores_tensor},
            "query_key"
        };
        first_matmul.set_attr<bool>(dnnl::graph::op::attr::transpose_b, true);
        graph_op scale_op{
            1,
            graph_op::kind::Multiply,
            {scores_tensor, scale_tensor},
            {scaled_tensor},
            "scale"
        };

        dnnl::graph::graph graph{engine.get_kind()};
        graph.add_op(first_matmul);
        graph.add_op(scale_op);

        auto softmax_input = scaled_tensor;
        std::vector<dnnl::graph::logical_tensor> inputs{query_tensor, key_tensor, scale_tensor};
        if (mask) {
            const auto mask_tensor =
                tensor_of(k_mask_id, mask->get().getDims(), to_graph_type(mask->get().getDtype()));
            const auto masked_tensor = tensor_of(k_masked_id, scores_dims, float_type::f32);
            graph.add_op(
                graph_op{
                    2,
                    graph_op::kind::Add,
                    {scaled_tensor, mask_tensor},
                    {masked_tensor},
                    "mask"
                }
            );
            inputs.push_back(mask_tensor);
            softmax_input = masked_tensor;
        }

        graph_op
            softmax{3, graph_op::kind::SoftMax, {softmax_input}, {probabilities_tensor}, "softmax"};
        softmax.set_attr<int64_t>(dnnl::graph::op::attr::axis, -1);
        graph_op second_matmul{
            4,
            graph_op::kind::MatMul,
            {probabilities_tensor, value_tensor},
            {output_tensor},
            "probabilities_value"
        };
        graph.add_op(softmax);
        graph.add_op(second_matmul);
        graph.finalize();

        auto partitions = graph.get_partitions();
        if (partitions.size() != 1 || !partitions.front().is_supported()) {
            throw std::runtime_error{"oneDNN graph did not fuse the SDPA pattern"};
        }
        inputs.push_back(value_tensor);
        auto compiled = partitions.front().compile(inputs, {output_tensor}, engine);
        for (auto& input: inputs) {
            input = compiled.query_logical_tensor(input.get_id());
        }
        return Entry{std::move(compiled), std::move(inputs)};
    }

    static std::string cache_key(
        const SyclTensor& query,
        const SyclTensor& key,
        std::optional<std::reference_wrapper<const SyclTensor>> mask,
        dnnl::graph::logical_tensor::data_type type
    )
    {
        std::string text = std::format("t{}", static_cast<int>(type));
        for (const auto* dims: {&query.getDims(), &key.getDims()}) {
            for (const auto value: *dims) {
                text += std::format(",{}", value);
            }
        }
        if (mask) {
            for (const auto value: mask->get().getDims()) {
                text += std::format("m{}", value);
            }
        }
        return text;
    }

    static inline std::map<std::string, Entry> s_cache;
};

} // namespace aten_xpu
