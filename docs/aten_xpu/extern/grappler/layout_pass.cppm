module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/tensor_properties.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:layout_pass;

import std;
import :graph_index;
import :pass_context;

export namespace aten_xpu {

// Converts channels-last (NHWC, NDHWC) graphs to the channels-first layout the plugin's kernels
// read, the way TensorFlow's layout optimizer converts for a GPU. Conv2D, Conv3D, BiasAdd and
// FusedBatchNorm nodes in a channels-last layout switch their data_format and get a Transpose in
// front of the data input; elementwise nodes that only read converted nodes (and scalars) follow;
// a Transpose back is added where a converted node feeds an unconverted one. A tensor that
// enters or leaves the converted region once shares one Transpose, so no pair needs cancelling.
// Preserved nodes and nodes whose rank or dtype the host's shape inference does not report stay.
class SyclLayoutPass
{
public:
    explicit SyclLayoutPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclLayoutPass(const SyclLayoutPass&) = delete;
    SyclLayoutPass& operator=(const SyclLayoutPass&) = delete;
    SyclLayoutPass(SyclLayoutPass&&) = delete;
    SyclLayoutPass& operator=(SyclLayoutPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        if (!m_context.getProperties().isInferred()) {
            return false;
        }
        m_scratch_new.clear();
        m_scratch_made.clear();
        bool changed = false;
        {
            const SyclGraphIndex index{graph};
            const auto count = static_cast<std::size_t>(index.size());
            m_scratch_rank.assign(count, 0);
            for (const int position: index.getTopologicalOrder()) {
                const int rank = convertedRank(graph, index, position);
                m_scratch_rank[static_cast<std::size_t>(position)] = rank;
            }
            for (int position = 0; position < index.size(); ++position) {
                if (m_scratch_rank[static_cast<std::size_t>(position)] != 0) {
                    convert(graph, index, position);
                    changed = true;
                }
            }
            for (int position = 0; position < index.size(); ++position) {
                if (m_scratch_rank[static_cast<std::size_t>(position)] == 0) {
                    convertBack(graph, index, position);
                }
            }
        }
        for (const auto& node: m_scratch_new) {
            *graph.add_node() = node;
        }
        return changed;
    }

private:
    static bool isSensitive(const std::string& op)
    {
        return op == "Conv2D" || op == "Conv3D" || op == "BiasAdd" || op == "FusedBatchNorm" ||
               op == "FusedBatchNormV2" || op == "FusedBatchNormV3";
    }

    static bool isAgnostic(const std::string& op)
    {
        return op == "Relu" || op == "Relu6" || op == "LeakyRelu" || op == "Sigmoid" ||
               op == "Tanh" || op == "Identity" || op == "Neg" || op == "Abs" || op == "Square" ||
               op == "Sqrt" || op == "Cast" || op == "Add" || op == "AddV2" || op == "Sub" ||
               op == "Mul" || op == "Maximum" || op == "Minimum" || op == "RealDiv";
    }

    static const char* channelsFirst(int rank)
    {
        return rank == 5 ? "NCDHW" : "NCHW";
    }

    static std::optional<int> channelsLastRank(const std::string& format)
    {
        if (format == "NHWC") {
            return 4;
        }
        if (format == "NDHWC") {
            return 5;
        }
        return std::nullopt;
    }

    std::optional<tensorflow::TensorProperties>
    outputOf(const tensorflow::GraphDef& graph, const SyclGraphIndex& index, int position, int slot)
    {
        const auto producer = index.dataInput(position, slot);
        if (!producer) {
            return std::nullopt;
        }
        return m_context.getProperties().output(
            graph.node(producer->first).name(),
            producer->second
        );
    }

    // The rank of the node's data output once converted, or 0 when the node stays.
    int convertedRank(const tensorflow::GraphDef& graph, const SyclGraphIndex& index, int position)
    {
        const tensorflow::NodeDef& node = index.node(position);
        if (m_context.isPreserved(node.name()) || index.hasControlEdges(position) ||
            node.input_size() == 0) {
            return 0;
        }
        if (isSensitive(node.op())) {
            return sensitiveRank(graph, index, position);
        }
        return isAgnostic(node.op()) ? followerRank(graph, index, position) : 0;
    }

    int sensitiveRank(const tensorflow::GraphDef& graph, const SyclGraphIndex& index, int position)
    {
        const tensorflow::NodeDef& node = index.node(position);
        const auto format = node.attr().find("data_format");
        if (format == node.attr().end()) {
            return 0;
        }
        const auto rank = channelsLastRank(format->second.s());
        const auto input = outputOf(graph, index, position, 0);
        if (!rank || !input || input->shape().unknown_rank() ||
            input->shape().dim_size() != *rank || input->dtype() == tensorflow::DT_INVALID) {
            return 0;
        }
        return *rank;
    }

    // Reads converted nodes and nothing but scalars besides.
    int followerRank(const tensorflow::GraphDef& graph, const SyclGraphIndex& index, int position)
    {
        int rank = 0;
        for (int slot = 0; slot < index.node(position).input_size(); ++slot) {
            const auto producer = index.dataInput(position, slot);
            if (!producer) {
                return 0;
            }
            const int converted = m_scratch_rank[static_cast<std::size_t>(producer->first)];
            if (converted != 0 && producer->second == 0 && (rank == 0 || rank == converted)) {
                rank = converted;
                continue;
            }
            const auto other = outputOf(graph, index, position, slot);
            if (!other || other->shape().unknown_rank() || other->shape().dim_size() != 0) {
                return 0;
            }
        }
        return rank;
    }

    void convert(tensorflow::GraphDef& graph, const SyclGraphIndex& index, int position)
    {
        tensorflow::NodeDef& node = *graph.mutable_node(position);
        const int rank = m_scratch_rank[static_cast<std::size_t>(position)];
        if (isSensitive(node.op())) {
            (*node.mutable_attr())["data_format"].set_s(channelsFirst(rank));
        }
        // Sensitive nodes read one data tensor; followers read converted tensors (or scalars).
        const int data_inputs = isSensitive(node.op()) ? 1 : node.input_size();
        for (int slot = 0; slot < data_inputs; ++slot) {
            const auto producer = index.dataInput(position, slot);
            if (!producer) {
                continue;
            }
            const bool converted = m_scratch_rank[static_cast<std::size_t>(producer->first)] != 0 &&
                                   producer->second == 0;
            const auto properties = outputOf(graph, index, position, slot);
            if (!converted && properties && properties->shape().dim_size() == rank) {
                node.set_input(slot, transposeOf(node.input(slot), *properties, rank, true));
            }
        }
    }

    // A converted node's output goes back to channels-last for the readers that stayed.
    void convertBack(tensorflow::GraphDef& graph, const SyclGraphIndex& index, int reader)
    {
        tensorflow::NodeDef& node = *graph.mutable_node(reader);
        for (int slot = 0; slot < node.input_size(); ++slot) {
            const auto producer = index.dataInput(reader, slot);
            if (!producer || producer->second != 0) {
                continue;
            }
            const int rank = m_scratch_rank[static_cast<std::size_t>(producer->first)];
            const auto properties = outputOf(graph, index, reader, slot);
            if (rank != 0 && properties) {
                node.set_input(slot, transposeOf(node.input(slot), *properties, rank, false));
            }
        }
    }

    // A Transpose of the tensor into channels-first (to_first) or back to channels-last, created
    // once and shared by every node that needs the same conversion.
    std::string transposeOf(
        const std::string& tensor,
        const tensorflow::TensorProperties& properties,
        int rank,
        bool to_first
    )
    {
        std::string name = "layout/transpose_" + tensor + (to_first ? "_first" : "_last");
        std::ranges::replace(name, ':', '_');
        if (!m_scratch_made.insert(name).second) {
            return name;
        }
        tensorflow::NodeDef transpose;
        transpose.set_name(name);
        transpose.set_op("Transpose");
        transpose.add_input(tensor);
        transpose.add_input(permutationOf(rank, to_first));
        (*transpose.mutable_attr())["T"].set_type(properties.dtype());
        (*transpose.mutable_attr())["Tperm"].set_type(tensorflow::DT_INT32);
        m_scratch_new.push_back(std::move(transpose));
        return name;
    }

    // The Const holding [0, 3, 1, 2] (NHWC to NCHW) or [0, 2, 3, 1], and the 5-d equivalents.
    std::string permutationOf(int rank, bool to_first)
    {
        const std::string name =
            "layout/perm_" + std::to_string(rank) + (to_first ? "_first" : "_last");
        if (!m_scratch_made.insert(name).second) {
            return name;
        }
        std::vector<int> order;
        if (to_first) {
            order.push_back(0);
            order.push_back(rank - 1);
            for (int axis = 1; axis < rank - 1; ++axis) {
                order.push_back(axis);
            }
        } else {
            order.push_back(0);
            for (int axis = 2; axis < rank; ++axis) {
                order.push_back(axis);
            }
            order.push_back(1);
        }
        tensorflow::NodeDef permutation;
        permutation.set_name(name);
        permutation.set_op("Const");
        (*permutation.mutable_attr())["dtype"].set_type(tensorflow::DT_INT32);
        auto* tensor = (*permutation.mutable_attr())["value"].mutable_tensor();
        tensor->set_dtype(tensorflow::DT_INT32);
        tensor->mutable_tensor_shape()->add_dim()->set_size(rank);
        for (const int axis: order) {
            tensor->add_int_val(axis);
        }
        m_scratch_new.push_back(std::move(permutation));
        return name;
    }

    SyclPassContext& m_context;
    std::vector<int> m_scratch_rank;
    std::set<std::string> m_scratch_made;
    std::vector<tensorflow::NodeDef> m_scratch_new;
};

} // namespace aten_xpu
