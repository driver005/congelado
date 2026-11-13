module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"

export module aten_xpu_extern_grappler:recomputation_pass;

import std;
import :graph_index;
import :graph_memory;
import :pass_context;
import :tensor_bytes;

export namespace aten_xpu {

// The recomputation heuristic of TensorFlow's memory optimizer. A cheap forward op (Add, Mul,
// Relu and similar, or one tagged with a _recompute_hint attr) whose output a gradient node reads
// is computed again for that node, so the forward value need not stay in memory until the
// gradient runs. Targets are the nodes in the gradients/ scope; the copies are named
// Recomputed/<node>, and a RecomputeTrigger NoOp, waiting on the gradient node's other inputs,
// keeps the copy from running before the backward pass reaches it. Cheap also means, when the
// host reports shapes and bandwidth, under 100 microseconds of memory traffic.
class SyclRecomputationPass
{
public:
    explicit SyclRecomputationPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclRecomputationPass(const SyclRecomputationPass&) = delete;
    SyclRecomputationPass& operator=(const SyclRecomputationPass&) = delete;
    SyclRecomputationPass(SyclRecomputationPass&&) = delete;
    SyclRecomputationPass& operator=(SyclRecomputationPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        m_scratch_new.clear();
        m_scratch_copies.clear();
        bool changed = false;
        {
            const SyclGraphIndex index{graph};
            for (int target = 0; target < index.size(); ++target) {
                if (inGradientScope(index.node(target).name())) {
                    changed |= recomputeInputs(graph, index, target);
                }
            }
        }
        for (const auto& node: m_scratch_new) {
            *graph.add_node() = node;
        }
        return changed;
    }

private:
    static constexpr std::string_view k_scope = "gradients/";
    static constexpr std::int64_t k_cost_threshold_ns = 100'000;

    static bool inGradientScope(const std::string& name)
    {
        return name.starts_with(k_scope) || name.contains("/" + std::string{k_scope});
    }

    static bool isCheapOp(const std::string& op)
    {
        return op == "Add" || op == "AddV2" || op == "Sub" || op == "Mul" || op == "Relu" ||
               op == "Relu6" || op == "Sigmoid" || op == "Tanh" || op == "Cast" || op == "Neg" ||
               op == "Square";
    }

    bool candidate(const SyclGraphIndex& index, int position)
    {
        const tensorflow::NodeDef& node = index.node(position);
        if (inGradientScope(node.name()) || m_context.isPreserved(node.name()) ||
            index.hasControlEdges(position)) {
            return false;
        }
        const bool hinted = node.attr().contains("_recompute_hint");
        if (!hinted && !isCheapOp(node.op())) {
            return false;
        }
        return hinted || cheapToMove(node);
    }

    // With shapes and a bandwidth, the node must move under 100 microseconds of data.
    bool cheapToMove(const tensorflow::NodeDef& node)
    {
        const std::uint64_t bandwidth = m_context.getDeviceBandwidth();
        const auto output = m_context.getProperties().output(node.name(), 0);
        const auto bytes = output ? m_bytes.of(*output) : std::nullopt;
        if (bandwidth == 0 || !bytes) {
            return true;
        }
        const auto nanoseconds =
            static_cast<double>(*bytes) * 1.0e9 / static_cast<double>(bandwidth);
        return nanoseconds < static_cast<double>(k_cost_threshold_ns);
    }

    bool recomputeInputs(tensorflow::GraphDef& graph, const SyclGraphIndex& index, int target)
    {
        bool changed = false;
        for (int slot = 0; slot < index.node(target).input_size(); ++slot) {
            const auto input = index.dataInput(target, slot);
            if (!input || !candidate(index, input->first)) {
                continue;
            }
            const std::string copy = copyOf(index, input->first, target);
            graph.mutable_node(target)->set_input(
                slot,
                input->second == 0 ? copy : copy + ":" + std::to_string(input->second)
            );
            changed = true;
        }
        return changed;
    }

    // Recomputed/<node>, with the cheap producers it reads copied too; shared by every target.
    std::string copyOf(const SyclGraphIndex& index, int position, int target)
    {
        const std::string name = "Recomputed/" + index.node(position).name();
        if (!m_scratch_copies.insert(name).second) {
            return name;
        }
        tensorflow::NodeDef copy = index.node(position);
        copy.set_name(name);
        for (int slot = 0; slot < copy.input_size(); ++slot) {
            const auto input = index.dataInput(position, slot);
            if (input && candidate(index, input->first)) {
                const std::string inner = copyOf(index, input->first, target);
                copy.set_input(
                    slot,
                    input->second == 0 ? inner : inner + ":" + std::to_string(input->second)
                );
            }
        }
        const std::string trigger = triggerFor(index, position, target);
        if (!trigger.empty()) {
            copy.add_input("^" + trigger);
        }
        m_scratch_new.push_back(std::move(copy));
        return name;
    }

    // A NoOp that waits on the other inputs of the gradient node: the recompute runs once the
    // backward pass has reached its operands, not at the start of the step.
    std::string triggerFor(const SyclGraphIndex& index, int recomputed, int target)
    {
        tensorflow::NodeDef trigger;
        trigger.set_name("RecomputeTrigger/" + index.node(recomputed).name());
        trigger.set_op("NoOp");
        for (int slot = 0; slot < index.node(target).input_size(); ++slot) {
            const auto input = index.dataInput(target, slot);
            if (input && input->first != recomputed && !candidate(index, input->first)) {
                trigger.add_input("^" + index.node(input->first).name());
            }
        }
        if (trigger.input_size() == 0) {
            return {};
        }
        const std::string name = trigger.name();
        m_scratch_new.push_back(std::move(trigger));
        return name;
    }

    SyclPassContext& m_context;
    SyclTensorBytes m_bytes;
    std::set<std::string> m_scratch_copies;
    std::vector<tensorflow::NodeDef> m_scratch_new;
};

} // namespace aten_xpu
