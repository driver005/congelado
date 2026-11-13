module;

#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"

export module aten_xpu_extern_grappler:dependency_pass;

import std;
import :graph_index;
import :graph_editor;
import :pass_context;

export namespace aten_xpu {

// Removes dependencies and nodes that only add ordering noise: duplicate control inputs, control
// inputs the node already has as data inputs, Identity nodes (reads go to the Identity's input),
// and NoOp nodes nothing depends on. Identity next to control flow ops, across devices or with
// control inputs stays, and so do preserved nodes.
class SyclDependencyPass
{
public:
    explicit SyclDependencyPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclDependencyPass(const SyclDependencyPass&) = delete;
    SyclDependencyPass& operator=(const SyclDependencyPass&) = delete;
    SyclDependencyPass(SyclDependencyPass&&) = delete;
    SyclDependencyPass& operator=(SyclDependencyPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        bool changed = false;
        {
            SyclGraphEditor editor{graph};
            const SyclGraphIndex index{graph};
            for (int position = 0; position < index.size(); ++position) {
                const tensorflow::NodeDef& node = index.node(position);
                if (m_context.isPreserved(node.name())) {
                    continue;
                }
                if (node.op() == "Identity" && passesThrough(index, position)) {
                    editor.bypass(node.name(), node.input(0));
                } else if (
                    node.op() == "NoOp" && !index.hasControlEdges(position) &&
                    node.input_size() == 0
                ) {
                    editor.remove(node.name());
                }
            }
            changed = editor.apply();
        }

        // After the bypasses, so a control input that now repeats a data input is caught too.
        for (auto& node: *graph.mutable_node()) {
            changed |= trimControlInputs(node);
        }
        return changed;
    }

private:
    bool trimControlInputs(tensorflow::NodeDef& node)
    {
        m_scratch_data.clear();
        for (const auto& input: node.input()) {
            const auto edge = SyclGraphIndex::Edge::parse(input);
            if (!edge.isControl()) {
                m_scratch_data.emplace(edge.getName());
            }
        }

        m_scratch_inputs.clear();
        m_scratch_seen.clear();
        for (const auto& input: node.input()) {
            const auto edge = SyclGraphIndex::Edge::parse(input);
            const bool redundant =
                edge.isControl() && (m_scratch_data.contains(std::string{edge.getName()}) ||
                                     !m_scratch_seen.emplace(edge.getName()).second);
            if (!redundant) {
                m_scratch_inputs.push_back(input);
            }
        }
        if (static_cast<int>(m_scratch_inputs.size()) == node.input_size()) {
            return false;
        }
        node.clear_input();
        for (const auto& input: m_scratch_inputs) {
            node.add_input(input);
        }
        return true;
    }

    bool passesThrough(const SyclGraphIndex& index, int position) const
    {
        const tensorflow::NodeDef& node = index.node(position);
        if (node.input_size() != 1) {
            return false;
        }
        const auto input = index.dataInput(position, 0);
        if (!input) {
            return false;
        }
        const tensorflow::NodeDef& producer = index.node(input->first);
        const std::string& op = producer.op();
        const bool control_flow = op == "Switch" || op == "RefSwitch" || op == "Merge" ||
                                  op == "Enter" || op == "Exit" || op == "NextIteration";
        const bool same_device = node.device().empty() || producer.device().empty() ||
                                 node.device() == producer.device();
        return !control_flow && same_device;
    }

    SyclPassContext& m_context;
    std::set<std::string> m_scratch_data;
    std::set<std::string> m_scratch_seen;
    std::vector<std::string> m_scratch_inputs;
};

} // namespace aten_xpu
