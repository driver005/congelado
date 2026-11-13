module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:auto_parallel_pass;

import std;
import :graph_index;
import :pass_context;

export namespace aten_xpu {

// TensorFlow's auto parallel: replicates the training step. The nodes the fetches depend on, minus
// variables and the shared input pipeline, are copied once per replica under
// AutoParallel-Replica-<i>/ and placed on /device:XPU:<i mod devices>; everything else is shared.
// Each gradient-applying op reads its gradient divided by the replica count
// (AutoParallel-Div-<op>), so the replicas together average. The fetches become NoOp nodes behind
// AutoParallel-Control-Fetch, which waits on every replica's fetch. Only the first dequeue op of
// the input pipeline is replicated, so each replica takes its own batch. A graph without fetch
// nodes or without a variable stays as it is, and so does one with fewer than two replicas.
class SyclAutoParallelPass
{
public:
    explicit SyclAutoParallelPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclAutoParallelPass(const SyclAutoParallelPass&) = delete;
    SyclAutoParallelPass& operator=(const SyclAutoParallelPass&) = delete;
    SyclAutoParallelPass(SyclAutoParallelPass&&) = delete;
    SyclAutoParallelPass& operator=(SyclAutoParallelPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        const int replicas = m_context.getReplicaCount();
        if (replicas < 2 || m_context.getFetch().empty()) {
            return false;
        }

        tensorflow::GraphDef output;
        {
            const SyclGraphIndex index{graph};
            markFanin(index);
            if (!hasVariable(index)) {
                return false;
            }
            markReplicated(index);
            *output.mutable_versions() = graph.versions();
            *output.mutable_library() = graph.library();
            addSharedNodes(index, output);
            addDivisor(output, replicas);
            for (int replica = 0; replica < replicas; ++replica) {
                addReplica(index, output, replica);
            }
            addFetches(index, output, replicas);
        }
        graph = std::move(output);
        return true;
    }

private:
    static constexpr std::string_view k_prefix = "AutoParallel-Replica-";

    // The position of the gradient input of each op that applies one, as in TensorFlow's table.
    static std::optional<int> gradientPosition(const std::string& op)
    {
        static const std::map<std::string, int> positions{
            {"ApplyGradientDescent", 2},
            {"ApplyProximalGradientDescent", 4},
            {"ApplyAdadelta", 6},
            {"ApplyAdagrad", 3},
            {"ApplyProximalAdagrad", 5},
            {"ApplyAdagradDA", 3},
            {"ApplyFtrl", 3},
            {"ApplyMomentum", 3},
            {"ApplyAdam", 9},
            {"ApplyRMSProp", 7},
            {"ApplyCenteredRMSProp", 8},
            {"ResourceApplyGradientDescent", 2},
            {"ResourceApplyAdam", 9},
            {"ResourceApplyRMSProp", 7}
        };
        const auto found = positions.find(op);
        return found == positions.end() ? std::nullopt : std::optional<int>{found->second};
    }

    static bool isVariable(const std::string& op)
    {
        return op == "Variable" || op == "VariableV2" || op == "VarHandleOp";
    }

    static bool isDequeue(const std::string& op)
    {
        return op.starts_with("QueueDequeue");
    }

    // Everything the fetch nodes depend on.
    void markFanin(const SyclGraphIndex& index)
    {
        m_scratch_training.assign(static_cast<std::size_t>(index.size()), false);
        m_scratch_work.clear();
        for (const auto& name: m_context.getFetch()) {
            const int position = index.find(name);
            if (position >= 0 && !m_scratch_training[static_cast<std::size_t>(position)]) {
                m_scratch_training[static_cast<std::size_t>(position)] = true;
                m_scratch_work.push_back(position);
            }
        }
        while (!m_scratch_work.empty()) {
            const int current = m_scratch_work.back();
            m_scratch_work.pop_back();
            for (const auto& input: index.node(current).input()) {
                const int producer = index.find(SyclGraphIndex::Edge::parse(input).getName());
                if (producer >= 0 && !m_scratch_training[static_cast<std::size_t>(producer)]) {
                    m_scratch_training[static_cast<std::size_t>(producer)] = true;
                    m_scratch_work.push_back(producer);
                }
            }
        }
    }

    bool hasVariable(const SyclGraphIndex& index) const
    {
        for (int position = 0; position < index.size(); ++position) {
            if (m_scratch_training[static_cast<std::size_t>(position)] &&
                isVariable(index.node(position).op())) {
                return true;
            }
        }
        return false;
    }

    // Replicated nodes: the training fan-in without variables and without the input pipeline in
    // front of the first dequeue (the dequeue itself is replicated).
    void markReplicated(const SyclGraphIndex& index)
    {
        const auto count = static_cast<std::size_t>(index.size());
        m_scratch_replicated.assign(count, false);
        m_scratch_input.assign(count, false);
        for (int position = 0; position < index.size(); ++position) {
            if (m_scratch_training[static_cast<std::size_t>(position)] &&
                isDequeue(index.node(position).op())) {
                markInputPipeline(index, position);
                break;
            }
        }
        for (int position = 0; position < index.size(); ++position) {
            const auto slot = static_cast<std::size_t>(position);
            m_scratch_replicated[slot] = m_scratch_training[slot] && !m_scratch_input[slot] &&
                                         !isVariable(index.node(position).op());
        }
    }

    void markInputPipeline(const SyclGraphIndex& index, int dequeue)
    {
        m_scratch_work.assign(1, dequeue);
        while (!m_scratch_work.empty()) {
            const int current = m_scratch_work.back();
            m_scratch_work.pop_back();
            for (const auto& input: index.node(current).input()) {
                const int producer = index.find(SyclGraphIndex::Edge::parse(input).getName());
                if (producer >= 0 && !m_scratch_input[static_cast<std::size_t>(producer)]) {
                    m_scratch_input[static_cast<std::size_t>(producer)] = true;
                    m_scratch_work.push_back(producer);
                }
            }
        }
    }

    std::string replicaName(const std::string& name, int replica) const
    {
        return std::string{k_prefix} + std::to_string(replica) + "/" + name;
    }

    // Reads of replicated nodes are renamed to the replica's copy; reads of shared nodes stay.
    std::string renamed(const SyclGraphIndex& index, const std::string& input, int replica) const
    {
        const auto edge = SyclGraphIndex::Edge::parse(input);
        const int producer = index.find(edge.getName());
        if (producer < 0 || !m_scratch_replicated[static_cast<std::size_t>(producer)]) {
            return input;
        }
        const std::string name = replicaName(std::string{edge.getName()}, replica);
        if (edge.isControl()) {
            return "^" + name;
        }
        return edge.getPort() == 0 ? name : name + ":" + std::to_string(edge.getPort());
    }

    void addSharedNodes(const SyclGraphIndex& index, tensorflow::GraphDef& output) const
    {
        for (int position = 0; position < index.size(); ++position) {
            if (m_scratch_replicated[static_cast<std::size_t>(position)]) {
                continue;
            }
            tensorflow::NodeDef* copy = output.add_node();
            *copy = index.node(position);
            // A shared node that reads a replicated one reads replica 0's copy.
            copy->clear_input();
            for (const auto& input: index.node(position).input()) {
                copy->add_input(renamed(index, input, 0));
            }
        }
    }

    static void addDivisor(tensorflow::GraphDef& output, int replicas)
    {
        tensorflow::NodeDef* divisor = output.add_node();
        divisor->set_name("AutoParallel-Div-Const");
        divisor->set_op("Const");
        (*divisor->mutable_attr())["dtype"].set_type(tensorflow::DT_FLOAT);
        auto* tensor = (*divisor->mutable_attr())["value"].mutable_tensor();
        tensor->set_dtype(tensorflow::DT_FLOAT);
        tensor->add_float_val(static_cast<float>(replicas));
    }

    void addReplica(const SyclGraphIndex& index, tensorflow::GraphDef& output, int replica) const
    {
        const int devices = m_context.getDeviceCount();
        for (int position = 0; position < index.size(); ++position) {
            if (!m_scratch_replicated[static_cast<std::size_t>(position)]) {
                continue;
            }
            const tensorflow::NodeDef& source = index.node(position);
            tensorflow::NodeDef* copy = output.add_node();
            *copy = source;
            copy->set_name(replicaName(source.name(), replica));
            copy->clear_input();
            for (const auto& input: source.input()) {
                copy->add_input(renamed(index, input, replica));
            }
            if (devices > 0) {
                copy->set_device("/device:XPU:" + std::to_string(replica % devices));
            }
            if (const auto gradient = gradientPosition(source.op());
                gradient && *gradient < copy->input_size()) {
                averageGradient(output, *copy, *gradient);
            }
        }
    }

    // The apply op reads its gradient divided by the replica count.
    static void
    averageGradient(tensorflow::GraphDef& output, tensorflow::NodeDef& apply, int position)
    {
        tensorflow::NodeDef* divide = output.add_node();
        divide->set_name("AutoParallel-Div-" + apply.name());
        divide->set_op("RealDiv");
        divide->add_input(apply.input(position));
        divide->add_input("AutoParallel-Div-Const");
        (*divide->mutable_attr())["T"].set_type(tensorflow::DT_FLOAT);
        apply.set_input(position, divide->name());
    }

    void addFetches(const SyclGraphIndex& index, tensorflow::GraphDef& output, int replicas) const
    {
        tensorflow::NodeDef* gate = output.add_node();
        gate->set_name("AutoParallel-Control-Fetch");
        gate->set_op("NoOp");
        for (const auto& name: m_context.getFetch()) {
            const int position = index.find(name);
            if (position < 0) {
                continue;
            }
            for (int replica = 0; replica < replicas; ++replica) {
                gate->add_input("^" + replicaName(name, replica));
            }
        }
        for (const auto& name: m_context.getFetch()) {
            const int position = index.find(name);
            if (position < 0 || !m_scratch_replicated[static_cast<std::size_t>(position)]) {
                continue;
            }
            tensorflow::NodeDef* fetch = output.add_node();
            fetch->set_name(name);
            fetch->set_op("NoOp");
            fetch->add_input("^" + gate->name());
        }
    }

    SyclPassContext& m_context;
    std::vector<bool> m_scratch_training;
    std::vector<bool> m_scratch_replicated;
    std::vector<bool> m_scratch_input;
    std::vector<int> m_scratch_work;
};

} // namespace aten_xpu
