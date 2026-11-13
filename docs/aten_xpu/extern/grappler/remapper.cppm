module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/tensor_properties.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:remapper;

import std;
import :graph_index;
import :activation;
import :properties;
import :op_lookup;
import :pass_context;

export namespace aten_xpu {

// Rewrites contraction + BiasAdd + activation chains into one fused node, the way TensorFlow's
// remapper does: it walks the graph in reverse topological order, matches from the outermost op
// up, checks types, fanout, control edges, shapes (graph properties) and that the host defines the
// fused op's attrs, and only then replaces the chain by a node that keeps the root's name.
class SyclRemapper
{
public:
    explicit SyclRemapper(SyclPassContext& context) noexcept :
        m_context{context},
        m_properties{context.getProperties()},
        m_lookup{context.getLookup()}
    {
    }

    SyclRemapper(const SyclRemapper&) = delete;
    SyclRemapper& operator=(const SyclRemapper&) = delete;
    SyclRemapper(SyclRemapper&&) = delete;
    SyclRemapper& operator=(SyclRemapper&&) = delete;

    // Replaces the graph by the remapped one; true when a chain was fused.
    bool run(tensorflow::GraphDef& graph)
    {
        const auto remapped = remap(graph);
        const bool changed = m_fused_count > 0;
        if (changed) {
            graph = remapped;
        }
        return changed;
    }

    tensorflow::GraphDef remap(const tensorflow::GraphDef& graph)
    {
        const SyclGraphIndex index{graph};
        m_removed.assign(static_cast<std::size_t>(index.size()), false);
        m_replacements.clear();
        m_fused_count = 0;

        const auto& order = index.getTopologicalOrder();
        for (auto position = order.rbegin(); position != order.rend(); ++position) {
            const int root = *position;
            if (m_removed[static_cast<std::size_t>(root)]) {
                continue;
            }
            if (!matchActivation(index, root)) {
                matchBias(index, root);
            }
        }

        tensorflow::GraphDef output;
        *output.mutable_versions() = graph.versions();
        *output.mutable_library() = graph.library();
        for (int position = 0; position < index.size(); ++position) {
            if (m_removed[static_cast<std::size_t>(position)]) {
                continue;
            }
            const auto replacement = m_replacements.find(position);
            *output.add_node() =
                replacement == m_replacements.end() ? index.node(position) : replacement->second;
        }
        return output;
    }

    std::size_t getFusedCount() const noexcept
    {
        return m_fused_count;
    }

private:
    // One bias add in a chain: the node and the input position that carries the bias.
    class Bias
    {
    public:
        int add{-1};
        int position{1};
    };

    // activation <- [bias add <-] contraction
    bool matchActivation(const SyclGraphIndex& index, int root)
    {
        const auto activation = m_activation.describe(index.node(root));
        if (!activation) {
            return false;
        }
        const int middle = soleProducer(index, root, 0);
        if (middle < 0) {
            return false;
        }
        if (isBiasAdd(index.node(middle))) {
            const auto contraction = contractionBehind(index, middle);
            return contraction && fuse(
                                      index,
                                      root,
                                      contraction->first,
                                      Bias{middle, contraction->second},
                                      activation
                                  );
        }
        return fuse(index, root, middle, std::nullopt, activation);
    }

    // bias add <- contraction
    bool matchBias(const SyclGraphIndex& index, int root)
    {
        if (!isBiasAdd(index.node(root))) {
            return false;
        }
        const auto contraction = contractionBehind(index, root);
        return contraction &&
               fuse(index, root, contraction->first, Bias{root, contraction->second}, std::nullopt);
    }

    // BiasAdd, or an Add that the shape check later proves to be a bias add.
    bool isBiasAdd(const tensorflow::NodeDef& node) const
    {
        return node.op() == "BiasAdd" || node.op() == "Add" || node.op() == "AddV2";
    }

    bool isContraction(const tensorflow::NodeDef& node) const
    {
        const std::string& op = node.op();
        return isConvolution(node) || op == "Linear" || op == "Addmm" || op == "Addmv" ||
               op == "Baddbmm" || op == "BatchMatMul";
    }

    bool isConvolution(const tensorflow::NodeDef& node) const
    {
        return node.op() == "Conv2D" || node.op() == "Conv3D";
    }

    // The contraction feeding a bias add and the input position of the bias. BiasAdd reads its
    // value first; Add and AddV2 are commutative, so either input may be the contraction.
    std::optional<std::pair<int, int>> contractionBehind(const SyclGraphIndex& index, int add) const
    {
        const int positions = index.node(add).op() == "BiasAdd" ? 1 : 2;
        for (int position = 0; position < positions; ++position) {
            const int producer = soleProducer(index, add, position);
            if (producer >= 0 && isContraction(index.node(producer))) {
                return std::pair<int, int>{producer, 1 - position};
            }
        }
        return std::nullopt;
    }

    // The producer of an input when only the reader uses it: one data edge, no control edges, and
    // not a node the caller must keep.
    int soleProducer(const SyclGraphIndex& index, int reader, int position) const
    {
        const auto input = index.dataInput(reader, position);
        if (!input || input->second != 0) {
            return -1;
        }
        const int producer = input->first;
        const bool owned = index.soleConsumer(producer, 0) == reader &&
                           index.dataFanout(producer, 0) == 1 && !index.hasControlEdges(producer) &&
                           !m_context.isPreserved(index.node(producer).name()) &&
                           !m_removed[static_cast<std::size_t>(producer)];
        return owned ? producer : -1;
    }

    bool fuse(
        const SyclGraphIndex& index,
        int root,
        int contraction,
        std::optional<Bias> bias,
        const std::optional<SyclFusedActivation::Description>& activation
    )
    {
        const tensorflow::NodeDef& source = index.node(contraction);
        const bool takes_bias = isConvolution(source) || source.op() == "Linear";
        const bool takes_code = source.op() == "Addmm" || source.op() == "Addmv" ||
                                source.op() == "Baddbmm" || source.op() == "BatchMatMul";
        if (!compatible(source) || (bias ? !takes_bias : !(takes_bias || takes_code))) {
            return false;
        }
        if (activation && !takes_bias && activation->code == 0) {
            return false;
        }
        if (!hasRoom(source, bias.has_value(), activation.has_value())) {
            return false;
        }
        if (bias && !biasMatches(index, contraction, *bias)) {
            return false;
        }
        if (!definedByHost(source.op(), bias.has_value(), activation.has_value(), takes_bias)) {
            return false;
        }

        tensorflow::NodeDef fused = source;
        fused.set_name(index.node(root).name());
        if (bias) {
            fused.add_input(index.node(bias->add).input(bias->position));
            const auto type = source.attr().at("T").type();
            (*fused.mutable_attr())["Textras"].mutable_list()->add_type(type);
        }
        if (activation) {
            setActivation(fused, *activation, takes_bias);
        }
        for (const auto& input: index.node(root).input()) {
            if (input.starts_with('^')) {
                fused.add_input(input);
            }
        }

        m_removed[static_cast<std::size_t>(contraction)] = true;
        if (bias && bias->add != root) {
            m_removed[static_cast<std::size_t>(bias->add)] = true;
        }
        m_replacements[root] = std::move(fused);
        ++m_fused_count;
        return true;
    }

    // The contraction runs on this plugin's device with a type the fused kernels support.
    bool compatible(const tensorflow::NodeDef& node) const
    {
        const auto type = node.attr().find("T");
        if (type == node.attr().end()) {
            return false;
        }
        const auto data_type = type->second.type();
        const bool supported = data_type == tensorflow::DT_FLOAT ||
                               data_type == tensorflow::DT_HALF ||
                               data_type == tensorflow::DT_BFLOAT16;
        std::string device = node.device();
        std::ranges::transform(
            device,
            device.begin(),
            [](unsigned char letter)
            {
                return static_cast<char>(std::toupper(letter));
            }
        );
        return supported && (device.empty() || device.contains("XPU"));
    }

    // The node has no bias input or activation yet that the fusion would stack on. A bias added
    // after an existing activation would run before it, so an activation blocks both fusions.
    bool hasRoom(const tensorflow::NodeDef& node, bool adds_bias, bool adds_activation) const
    {
        if (adds_bias) {
            const auto extras = node.attr().find("Textras");
            if (extras != node.attr().end() && extras->second.list().type_size() > 0) {
                return false;
            }
        }
        if (adds_bias || adds_activation) {
            const auto unary = node.attr().find("unary");
            if (unary != node.attr().end() && !unary->second.s().empty() &&
                unary->second.s() != "none") {
                return false;
            }
            const auto code = node.attr().find("activation");
            if (code != node.attr().end() && code->second.i() != 0) {
                return false;
            }
        }
        return true;
    }

    // The bias is a vector along the output channels of the contraction. An Add is only a bias add
    // when the shapes prove it, so it needs the host's shape inference and a Linear contraction.
    bool biasMatches(const SyclGraphIndex& index, int contraction, const Bias& bias_use)
    {
        const tensorflow::NodeDef& add = index.node(bias_use.add);
        const tensorflow::NodeDef& source = index.node(contraction);
        const auto source_type = source.attr().at("T").type();
        const auto add_type = add.attr().find("T");
        if (add_type == add.attr().end() || add_type->second.type() != source_type ||
            add.input_size() < 2) {
            return false;
        }

        const bool plain = add.op() == "BiasAdd";
        if (plain) {
            const auto format = add.attr().find("data_format");
            const std::string layout = format == add.attr().end() ? "NHWC" : format->second.s();
            const char* expected = !isConvolution(source)    ? "NHWC"
                                   : source.op() == "Conv3D" ? "NCDHW"
                                                             : "NCHW";
            if (layout != expected) {
                return false;
            }
        } else if (!m_properties.isInferred()) {
            return false;
        }
        if (!m_properties.isInferred()) {
            return true;
        }

        const auto bias = m_properties.input(add.name(), bias_use.position);
        const auto weight = m_properties.input(source.name(), 1);
        if (!bias || !weight || bias->shape().unknown_rank() || weight->shape().unknown_rank() ||
            weight->shape().dim_size() < 1) {
            return false;
        }
        const auto channels = weight->shape().dim(0).size();
        if (!plain && isConvolution(source)) {
            return channelShaped(*bias, weight->shape().dim_size(), channels);
        }
        if (bias->shape().dim_size() != 1) {
            return false;
        }
        const auto bias_size = bias->shape().dim(0).size();
        if (!plain) {
            return source.op() == "Linear" && bias_size >= 0 && bias_size == channels;
        }
        return bias_size < 0 || channels < 0 || bias_size == channels;
    }

    // [1, channels, 1, ...] with the rank of the convolution output: the same elements as a bias
    // vector, which the kernel reads by element count.
    bool channelShaped(
        const tensorflow::TensorProperties& bias,
        int output_rank,
        std::int64_t channels
    ) const
    {
        if (bias.shape().dim_size() != output_rank || output_rank < 2) {
            return false;
        }
        for (int axis = 0; axis < output_rank; ++axis) {
            const auto size = bias.shape().dim(axis).size();
            if (axis == 1 ? (size < 0 || size != channels) : size != 1) {
                return false;
            }
        }
        return true;
    }

    // The host defines the attrs and inputs the fused node carries.
    bool definedByHost(std::string_view op, bool adds_bias, bool adds_activation, bool takes_bias)
    {
        if (!m_lookup.available()) {
            return true;
        }
        if (adds_bias && !m_lookup.defines(op, {"Textras"}, {"extras"})) {
            return false;
        }
        if (adds_activation) {
            return takes_bias ? m_lookup.defines(op, {"unary"}, {})
                              : m_lookup.defines(op, {"activation"}, {});
        }
        return true;
    }

    void setActivation(
        tensorflow::NodeDef& node,
        const SyclFusedActivation::Description& activation,
        bool by_name
    ) const
    {
        auto& attributes = *node.mutable_attr();
        if (!by_name) {
            attributes["activation"].set_i(activation.code);
            return;
        }
        attributes["unary"].set_s(activation.unary);
        if (!activation.algorithm.empty()) {
            attributes["algorithm"].set_s(activation.algorithm);
        }
        if (!activation.scalars.empty()) {
            auto* scalars = attributes["scalars"].mutable_list();
            scalars->clear_f();
            for (const float value: activation.scalars) {
                scalars->add_f(value);
            }
        }
    }

    SyclPassContext& m_context;
    SyclGraphProperties& m_properties;
    SyclOpLookup& m_lookup;
    SyclFusedActivation m_activation;
    std::vector<bool> m_removed;
    std::map<int, tensorflow::NodeDef> m_replacements;
    std::size_t m_fused_count{0};
};

} // namespace aten_xpu
