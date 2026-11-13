module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"

export module aten_xpu_extern_grappler:batch_norm_folding_pass;

import std;
import :graph_index;
import :const_tensor;
import :pass_context;

export namespace aten_xpu {

// Folds an inference FusedBatchNorm that reads a convolution into the convolution: the weights
// are scaled per output channel and the shift becomes a bias, so the batch norm node turns into a
// BiasAdd (which the remapper then fuses) and no batch norm runs. Needs float Const weights and
// Const scale, offset, mean and variance, a convolution nothing else reads, and no use of the
// batch norm's statistics outputs.
class SyclBatchNormFoldingPass
{
public:
    explicit SyclBatchNormFoldingPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclBatchNormFoldingPass(const SyclBatchNormFoldingPass&) = delete;
    SyclBatchNormFoldingPass& operator=(const SyclBatchNormFoldingPass&) = delete;
    SyclBatchNormFoldingPass(SyclBatchNormFoldingPass&&) = delete;
    SyclBatchNormFoldingPass& operator=(SyclBatchNormFoldingPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        m_folded.clear();
        {
            const SyclGraphIndex index{graph};
            for (int position = 0; position < index.size(); ++position) {
                fold(graph, index, position);
            }
        }
        for (const auto& added: m_folded) {
            tensorflow::NodeDef* node = graph.add_node();
            node->set_name(added.first);
            added.second.writeTo(*node);
        }
        return !m_folded.empty();
    }

private:
    void fold(tensorflow::GraphDef& graph, const SyclGraphIndex& index, int batch_norm)
    {
        const tensorflow::NodeDef& norm = index.node(batch_norm);
        if (!isBatchNorm(norm) || norm.input_size() != 5 || index.hasControlEdges(batch_norm)) {
            return;
        }
        for (int port = 1; port < 6; ++port) {
            if (index.dataFanout(batch_norm, port) != 0) {
                return;
            }
        }
        const auto training = norm.attr().find("is_training");
        if (training == norm.attr().end() || training->second.b()) {
            return;
        }

        const auto source = index.dataInput(batch_norm, 0);
        if (!source || source->second != 0) {
            return;
        }
        const int convolution = source->first;
        const tensorflow::NodeDef& conv = index.node(convolution);
        if (!ownsOutput(index, convolution, batch_norm) || !hasNoFusion(conv) ||
            !layoutMatches(norm, conv)) {
            return;
        }

        const auto weight = constantAt(graph, index, convolution, 1);
        const auto scale = constantAt(graph, index, batch_norm, 1);
        const auto offset = constantAt(graph, index, batch_norm, 2);
        const auto mean = constantAt(graph, index, batch_norm, 3);
        const auto variance = constantAt(graph, index, batch_norm, 4);
        if (!weight || !scale || !offset || !mean || !variance || weight->getDims().size() < 2) {
            return;
        }
        const auto channels = static_cast<std::size_t>(weight->getDims().front());
        for (const auto* vector: {&*scale, &*offset, &*mean, &*variance}) {
            if (vector->getDims().size() != 1 || vector->getElementCount() != channels) {
                return;
            }
        }

        const std::string weight_name = norm.name() + "/folded_weight";
        const std::string bias_name = norm.name() + "/folded_bias";
        if (index.find(weight_name) >= 0 || index.find(bias_name) >= 0) {
            return;
        }

        const auto epsilon = norm.attr().find("epsilon");
        const float shift = epsilon == norm.attr().end() ? 1.0e-4F : epsilon->second.f();
        const std::size_t per_channel = channels == 0 ? 0 : weight->getElementCount() / channels;
        std::vector<float> weights(weight->getValues());
        std::vector<float> biases(channels);
        for (std::size_t channel = 0; channel < channels; ++channel) {
            const float factor =
                scale->getValues()[channel] / std::sqrt(variance->getValues()[channel] + shift);
            for (std::size_t element = 0; element < per_channel; ++element) {
                weights[channel * per_channel + element] *= factor;
            }
            biases[channel] = offset->getValues()[channel] - mean->getValues()[channel] * factor;
        }
        m_folded.emplace_back(weight_name, SyclConstTensor{weight->getDims(), std::move(weights)});
        m_folded.emplace_back(
            bias_name,
            SyclConstTensor{{static_cast<std::int64_t>(channels)}, std::move(biases)}
        );

        // The convolution reads the folded weights; the batch norm becomes the bias add.
        graph.mutable_node(convolution)->set_input(1, weight_name);
        tensorflow::NodeDef& rewritten = *graph.mutable_node(batch_norm);
        const auto type = norm.attr().at("T");
        const auto format = norm.attr().at("data_format");
        rewritten.set_op("BiasAdd");
        rewritten.clear_input();
        rewritten.add_input(conv.name());
        rewritten.add_input(bias_name);
        rewritten.mutable_attr()->clear();
        (*rewritten.mutable_attr())["T"] = type;
        (*rewritten.mutable_attr())["data_format"] = format;
    }

    static bool isBatchNorm(const tensorflow::NodeDef& node)
    {
        return node.op() == "FusedBatchNorm" || node.op() == "FusedBatchNormV2" ||
               node.op() == "FusedBatchNormV3";
    }

    bool ownsOutput(const SyclGraphIndex& index, int producer, int reader) const
    {
        return index.soleConsumer(producer, 0) == reader && index.dataFanout(producer, 0) == 1 &&
               !index.hasControlEdges(producer) &&
               !m_context.isPreserved(index.node(producer).name());
    }

    // A convolution without a bias or an activation yet: a batch norm after an activation cannot
    // move before it.
    bool hasNoFusion(const tensorflow::NodeDef& conv) const
    {
        if (conv.op() != "Conv2D" && conv.op() != "Conv3D") {
            return false;
        }
        const auto extras = conv.attr().find("Textras");
        if (extras != conv.attr().end() && extras->second.list().type_size() > 0) {
            return false;
        }
        const auto unary = conv.attr().find("unary");
        if (unary != conv.attr().end() && !unary->second.s().empty() &&
            unary->second.s() != "none") {
            return false;
        }
        const auto code = conv.attr().find("activation");
        return conv.input_size() == 2 && (code == conv.attr().end() || code->second.i() == 0);
    }

    bool layoutMatches(const tensorflow::NodeDef& norm, const tensorflow::NodeDef& conv) const
    {
        const auto format = norm.attr().find("data_format");
        const auto type = norm.attr().find("T");
        if (format == norm.attr().end() || type == norm.attr().end()) {
            return false;
        }
        return format->second.s() == (conv.op() == "Conv3D" ? "NCDHW" : "NCHW");
    }

    std::optional<SyclConstTensor> constantAt(
        const tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        int position,
        int slot
    ) const
    {
        const auto input = index.dataInput(position, slot);
        if (!input || input->second != 0) {
            return std::nullopt;
        }
        return SyclConstTensor::fromNode(graph.node(input->first));
    }

    SyclPassContext& m_context;
    std::vector<std::pair<std::string, SyclConstTensor>> m_folded;
};

} // namespace aten_xpu
