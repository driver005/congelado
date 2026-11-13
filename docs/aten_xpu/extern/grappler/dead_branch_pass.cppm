module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:dead_branch_pass;

import std;
import :graph_index;
import :graph_editor;
import :pass_context;

export namespace aten_xpu {

// The dead branch part of TensorFlow's loop optimizer: a Switch whose predicate is a Const bool
// has one live output. Reads of the live output go to the Switch's data input, everything that
// only the dead output reaches is removed, and a Merge left with one live input is bypassed. A
// Switch is left alone when the dead region reaches a preserved node, a loop op (Enter, Exit,
// NextIteration, LoopCond), or a Merge that would keep several live inputs. A while loop whose
// condition compares constants and is false on entry has a dead body too: it never runs, so its
// NextIteration nodes may go as well and the loop variables keep their entering values.
class SyclDeadBranchPass
{
public:
    explicit SyclDeadBranchPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclDeadBranchPass(const SyclDeadBranchPass&) = delete;
    SyclDeadBranchPass& operator=(const SyclDeadBranchPass&) = delete;
    SyclDeadBranchPass(SyclDeadBranchPass&&) = delete;
    SyclDeadBranchPass& operator=(SyclDeadBranchPass&&) = delete;

    // One Switch per round, because removing it changes the graph the next one is found in.
    bool run(tensorflow::GraphDef& graph)
    {
        bool changed = false;
        for (int round = 0; round <= graph.node_size(); ++round) {
            if (!foldOneSwitch(graph)) {
                break;
            }
            changed = true;
        }
        return changed;
    }

private:
    bool foldOneSwitch(tensorflow::GraphDef& graph)
    {
        SyclGraphEditor editor{graph};
        {
            const SyclGraphIndex index{graph};
            for (int position = 0; position < index.size(); ++position) {
                if (tryFold(graph, index, position, editor)) {
                    break;
                }
            }
        }
        return editor.apply();
    }

    bool tryFold(
        const tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        int branch,
        SyclGraphEditor& editor
    )
    {
        const tensorflow::NodeDef& node = index.node(branch);
        if (node.op() != "Switch" || node.input_size() != 2 || index.hasControlEdges(branch) ||
            m_context.isPreserved(node.name())) {
            return false;
        }
        auto predicate = constantBool(graph, index, branch, 1);
        m_zero_iteration = false;
        if (!predicate) {
            // A loop condition that is false on entry: the body never runs.
            predicate = firstIterationCondition(graph, index, branch);
            m_zero_iteration = predicate.has_value();
            if (predicate && *predicate) {
                return false;
            }
        }
        const auto data = index.dataInput(branch, 0);
        if (!predicate || !data) {
            return false;
        }

        // Switch outputs: port 0 is taken when the predicate is false, port 1 when it is true.
        const int live_port = *predicate ? 1 : 0;
        markDead(index, branch, 1 - live_port);
        if (!deadRegionIsSafe(index)) {
            return false;
        }

        editor.redirect(node.name(), live_port, node.input(0));
        editor.remove(node.name());
        for (int position = 0; position < index.size(); ++position) {
            if (m_scratch_dead[static_cast<std::size_t>(position)]) {
                editor.remove(index.node(position).name());
            }
        }
        for (const auto& [merge, live_input]: m_scratch_merges) {
            editor.bypass(index.node(merge).name(), index.node(merge).input(live_input));
        }
        return true;
    }

    std::optional<bool> constantBool(
        const tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        int position,
        int slot
    ) const
    {
        const auto input = index.dataInput(position, slot);
        if (!input) {
            return std::nullopt;
        }
        const tensorflow::NodeDef& source = graph.node(input->first);
        const auto type = source.attr().find("dtype");
        const auto value = source.attr().find("value");
        if (source.op() != "Const" || type == source.attr().end() || value == source.attr().end() ||
            type->second.type() != tensorflow::DT_BOOL ||
            value->second.tensor().bool_val_size() != 1 ||
            value->second.tensor().tensor_shape().dim_size() != 0) {
            return std::nullopt;
        }
        return value->second.tensor().bool_val(0);
    }

    // The condition of a while loop on its first iteration, when it compares constants: LoopCond
    // over Less, LessEqual, Greater, GreaterEqual, Equal or NotEqual, whose operands trace back
    // through Merge (the entering value), Enter and Identity to scalar Const nodes.
    std::optional<bool> firstIterationCondition(
        const tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        int branch
    ) const
    {
        const auto cond = index.dataInput(branch, 1);
        if (!cond || graph.node(cond->first).op() != "LoopCond") {
            return std::nullopt;
        }
        const auto compare = index.dataInput(cond->first, 0);
        if (!compare || index.node(compare->first).input_size() != 2) {
            return std::nullopt;
        }
        const std::string& op = index.node(compare->first).op();
        const auto left = initialScalar(graph, index, compare->first, 0, 0);
        const auto right = initialScalar(graph, index, compare->first, 1, 0);
        if (!left || !right) {
            return std::nullopt;
        }
        if (op == "Less") {
            return *left < *right;
        }
        if (op == "LessEqual") {
            return *left <= *right;
        }
        if (op == "Greater") {
            return *left > *right;
        }
        if (op == "GreaterEqual") {
            return *left >= *right;
        }
        if (op == "Equal") {
            return *left == *right;
        }
        if (op == "NotEqual") {
            return *left != *right;
        }
        return std::nullopt;
    }

    // The value a scalar input holds when the loop is entered.
    std::optional<double> initialScalar(
        const tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        int position,
        int slot,
        int depth
    ) const
    {
        const auto input = index.dataInput(position, slot);
        if (!input || depth > k_max_trace_depth) {
            return std::nullopt;
        }
        const tensorflow::NodeDef& node = graph.node(input->first);
        if (node.op() == "Const") {
            return scalarValue(node);
        }
        if (node.op() == "Merge") {
            return initialScalar(graph, index, input->first, 0, depth + 1);
        }
        if (node.op() == "Enter" || node.op() == "Identity") {
            return initialScalar(graph, index, input->first, 0, depth + 1);
        }
        return std::nullopt;
    }

    static std::optional<double> scalarValue(const tensorflow::NodeDef& node)
    {
        const auto type = node.attr().find("dtype");
        const auto value = node.attr().find("value");
        if (type == node.attr().end() || value == node.attr().end() ||
            value->second.tensor().tensor_shape().dim_size() != 0) {
            return std::nullopt;
        }
        const auto& tensor = value->second.tensor();
        switch (type->second.type()) {
            case tensorflow::DT_FLOAT:
                return tensor.float_val_size() == 1 ? std::optional<double>{tensor.float_val(0)}
                                                    : std::nullopt;
            case tensorflow::DT_INT32:
                return tensor.int_val_size() == 1 ? std::optional<double>{tensor.int_val(0)}
                                                  : std::nullopt;
            case tensorflow::DT_INT64:
                return tensor.int64_val_size() == 1
                           ? std::optional<double>{static_cast<double>(tensor.int64_val(0))}
                           : std::nullopt;
            default:
                return std::nullopt;
        }
    }

    // Marks every node that only the dead output reaches. A Merge dies when all its inputs are
    // dead; with some dead inputs it is recorded (with the live input position) for a bypass.
    void markDead(const SyclGraphIndex& index, int branch, int dead_port)
    {
        const auto count = static_cast<std::size_t>(index.size());
        m_scratch_dead.assign(count, false);
        m_scratch_merges.clear();
        m_scratch_blocked = false;

        bool grew = true;
        while (grew) {
            grew = false;
            m_scratch_merges.clear();
            for (int position = 0; position < index.size(); ++position) {
                if (position == branch || m_scratch_dead[static_cast<std::size_t>(position)]) {
                    continue;
                }
                const tensorflow::NodeDef& node = index.node(position);
                int dead_inputs = 0;
                int live_inputs = 0;
                int live_slot = -1;
                for (int slot = 0; slot < node.input_size(); ++slot) {
                    const auto edge = SyclGraphIndex::Edge::parse(node.input(slot));
                    const int producer = index.find(edge.getName());
                    const bool dead =
                        producer >= 0 &&
                        ((producer == branch && !edge.isControl() && edge.getPort() == dead_port) ||
                         m_scratch_dead[static_cast<std::size_t>(producer)]);
                    if (edge.isControl()) {
                        dead_inputs += dead ? 1 : 0;
                    } else if (dead) {
                        ++dead_inputs;
                    } else {
                        ++live_inputs;
                        live_slot = slot;
                    }
                }
                if (dead_inputs == 0) {
                    continue;
                }
                if (node.op() == "Merge") {
                    if (live_inputs == 0) {
                        m_scratch_dead[static_cast<std::size_t>(position)] = true;
                        grew = true;
                    } else if (live_inputs == 1 && index.dataFanout(position, 1) == 0) {
                        m_scratch_merges.emplace_back(position, live_slot);
                    } else {
                        m_scratch_blocked = true;
                    }
                } else {
                    m_scratch_dead[static_cast<std::size_t>(position)] = true;
                    grew = true;
                }
            }
        }
    }

    bool deadRegionIsSafe(const SyclGraphIndex& index) const
    {
        if (m_scratch_blocked) {
            return false;
        }
        for (int position = 0; position < index.size(); ++position) {
            if (!m_scratch_dead[static_cast<std::size_t>(position)]) {
                continue;
            }
            const tensorflow::NodeDef& node = index.node(position);
            const std::string& op = node.op();
            // A loop that never runs has its NextIteration nodes in the dead body.
            const bool loop_op = op == "Enter" || op == "Exit" || op == "LoopCond" ||
                                 (op == "NextIteration" && !m_zero_iteration);
            if (m_context.isPreserved(node.name()) || loop_op) {
                return false;
            }
        }
        return true;
    }

    static constexpr int k_max_trace_depth = 8;

    SyclPassContext& m_context;
    bool m_zero_iteration{false};
    std::vector<bool> m_scratch_dead;
    std::vector<std::pair<int, int>> m_scratch_merges;
    bool m_scratch_blocked{false};
};

} // namespace aten_xpu
