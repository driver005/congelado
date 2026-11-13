module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/tensor_properties.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:mixed_precision_pass;

import std;
import :graph_index;
import :pass_context;

export namespace aten_xpu {

// Runs the contractions in half or bfloat16, as TensorFlow's auto mixed precision does. Conv2D,
// Conv3D, Linear, Addmm, Addmv, Baddbmm and BatchMatMul nodes whose inputs the host's shape
// inference reports as float32 switch to the low type; Relu, Relu6, Sigmoid, Tanh and Identity
// nodes that only read converted nodes follow, so chains stay in low precision. Casts go where a
// float32 tensor enters a converted node or a converted node feeds an unconverted one, and a cast
// is shared by every node that reads the same tensor. Preserved nodes keep float32.
class SyclMixedPrecisionPass
{
public:
    explicit SyclMixedPrecisionPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclMixedPrecisionPass(const SyclMixedPrecisionPass&) = delete;
    SyclMixedPrecisionPass& operator=(const SyclMixedPrecisionPass&) = delete;
    SyclMixedPrecisionPass(SyclMixedPrecisionPass&&) = delete;
    SyclMixedPrecisionPass& operator=(SyclMixedPrecisionPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        if (!m_context.getProperties().isInferred()) {
            return false;
        }
        const auto low =
            m_context.prefersBfloat16() ? tensorflow::DT_BFLOAT16 : tensorflow::DT_HALF;
        m_scratch_new.clear();
        m_scratch_casts.clear();
        bool changed = false;
        {
            const SyclGraphIndex index{graph};
            const auto count = static_cast<std::size_t>(index.size());
            m_scratch_converted.assign(count, false);
            for (const int position: index.getTopologicalOrder()) {
                if (convertible(graph, index, position)) {
                    m_scratch_converted[static_cast<std::size_t>(position)] = true;
                }
            }
            for (int position = 0; position < index.size(); ++position) {
                if (m_scratch_converted[static_cast<std::size_t>(position)]) {
                    convert(graph, index, position, low);
                    changed = true;
                }
            }
            for (int position = 0; position < index.size(); ++position) {
                if (!m_scratch_converted[static_cast<std::size_t>(position)]) {
                    castOutputsBack(graph, index, position, low);
                }
            }
        }
        for (const auto& node: m_scratch_new) {
            *graph.add_node() = node;
        }
        return changed;
    }

private:
    static bool isContraction(const std::string& op)
    {
        return op == "Conv2D" || op == "Conv3D" || op == "Linear" || op == "Addmm" ||
               op == "Addmv" || op == "Baddbmm" || op == "BatchMatMul";
    }

    static bool isFollower(const std::string& op)
    {
        return op == "Relu" || op == "Relu6" || op == "Sigmoid" || op == "Tanh" || op == "Identity";
    }

    // The element type of a node's data input: a Const's own dtype, or the type the host's shape
    // inference reports for the producer's output (which stays valid for nodes an earlier pass
    // fused, since the fused node keeps the name and the type).
    std::optional<tensorflow::DataType> inputType(
        const tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        int position,
        int slot
    )
    {
        const auto producer = index.dataInput(position, slot);
        if (!producer) {
            return std::nullopt;
        }
        const tensorflow::NodeDef& source = graph.node(producer->first);
        if (source.op() == "Const") {
            const auto type = source.attr().find("dtype");
            return type == source.attr().end() ? std::nullopt : std::optional{type->second.type()};
        }
        const auto properties = m_context.getProperties().output(source.name(), producer->second);
        return properties ? std::optional{properties->dtype()} : std::nullopt;
    }

    bool floatInputs(const tensorflow::GraphDef& graph, const SyclGraphIndex& index, int position)
    {
        const tensorflow::NodeDef& node = index.node(position);
        const auto type = node.attr().find("T");
        if (type == node.attr().end() || type->second.type() != tensorflow::DT_FLOAT) {
            return false;
        }
        for (int slot = 0; slot < node.input_size(); ++slot) {
            if (node.input(slot).starts_with('^')) {
                continue;
            }
            const auto input = inputType(graph, index, position, slot);
            if (!input || *input != tensorflow::DT_FLOAT) {
                return false;
            }
        }
        return node.input_size() > 0;
    }

    bool convertible(const tensorflow::GraphDef& graph, const SyclGraphIndex& index, int position)
    {
        const tensorflow::NodeDef& node = index.node(position);
        if (m_context.isPreserved(node.name()) || index.hasControlEdges(position) ||
            !floatInputs(graph, index, position)) {
            return false;
        }
        if (isContraction(node.op())) {
            return true;
        }
        if (!isFollower(node.op()) || node.input_size() != 1) {
            return false;
        }
        const auto input = index.dataInput(position, 0);
        return input && input->second == 0 &&
               m_scratch_converted[static_cast<std::size_t>(input->first)];
    }

    // Switches the node to the low type and casts every input that is not already low.
    void convert(
        tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        int position,
        tensorflow::DataType low
    )
    {
        tensorflow::NodeDef& node = *graph.mutable_node(position);
        (*node.mutable_attr())["T"].set_type(low);
        const auto extras = node.mutable_attr()->find("Textras");
        if (extras != node.mutable_attr()->end()) {
            const int count = extras->second.list().type_size();
            extras->second.mutable_list()->clear_type();
            for (int slot = 0; slot < count; ++slot) {
                extras->second.mutable_list()->add_type(low);
            }
        }
        for (int slot = 0; slot < node.input_size(); ++slot) {
            const auto producer = index.dataInput(position, slot);
            const bool already_low = producer && producer->second == 0 &&
                                     m_scratch_converted[static_cast<std::size_t>(producer->first)];
            if (!already_low && !node.input(slot).starts_with('^')) {
                node.set_input(slot, castOf(node.input(slot), tensorflow::DT_FLOAT, low));
            }
        }
    }

    // Unconverted readers of a converted node get a float32 cast of its output.
    void castOutputsBack(
        tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        int reader,
        tensorflow::DataType low
    )
    {
        tensorflow::NodeDef& node = *graph.mutable_node(reader);
        for (int slot = 0; slot < node.input_size(); ++slot) {
            const auto producer = index.dataInput(reader, slot);
            if (producer && producer->second == 0 &&
                m_scratch_converted[static_cast<std::size_t>(producer->first)]) {
                node.set_input(slot, castOf(node.input(slot), low, tensorflow::DT_FLOAT));
            }
        }
    }

    // The name of a cast node for the tensor, created on first use and shared afterwards.
    std::string
    castOf(const std::string& tensor, tensorflow::DataType from, tensorflow::DataType to)
    {
        std::string key = tensor + "->" + std::to_string(static_cast<int>(to));
        const auto known = m_scratch_casts.find(key);
        if (known != m_scratch_casts.end()) {
            return known->second;
        }

        std::string name = "amp_cast/" + tensor + "_to_" + std::to_string(static_cast<int>(to));
        std::ranges::replace(name, ':', '_');
        tensorflow::NodeDef cast;
        cast.set_name(name);
        cast.set_op("Cast");
        cast.add_input(tensor);
        (*cast.mutable_attr())["SrcT"].set_type(from);
        (*cast.mutable_attr())["DstT"].set_type(to);
        (*cast.mutable_attr())["Truncate"].set_b(false);
        m_scratch_new.push_back(std::move(cast));
        m_scratch_casts.emplace(std::move(key), name);
        return name;
    }

    SyclPassContext& m_context;
    std::vector<bool> m_scratch_converted;
    std::vector<tensorflow::NodeDef> m_scratch_new;
    std::map<std::string, std::string> m_scratch_casts;
};

} // namespace aten_xpu
