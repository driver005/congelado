module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/tensor_properties.pb.h"

export module aten_xpu_extern_grappler:loop_invariant_motion_pass;

import std;
import :graph_index;
import :frame_view;
import :pass_context;

export namespace aten_xpu {

// Moves loop invariant computation out of loops, as TensorFlow's loop optimizer does. In an
// outermost loop frame, a node whose inputs are all constant Enter nodes, Const nodes or other
// invariant nodes computes the same value every iteration, so it is rewired to read the Enter's
// input (outside the loop) and runs once; the loop reads its result through a new constant Enter.
// Only ops the host defines as stateless move, nodes that a variant node waits on through a control
// edge stay, and nested loops are left alone. The result type comes from the host's shape
// inference.
class SyclLoopInvariantMotionPass
{
public:
    explicit SyclLoopInvariantMotionPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclLoopInvariantMotionPass(const SyclLoopInvariantMotionPass&) = delete;
    SyclLoopInvariantMotionPass& operator=(const SyclLoopInvariantMotionPass&) = delete;
    SyclLoopInvariantMotionPass(SyclLoopInvariantMotionPass&&) = delete;
    SyclLoopInvariantMotionPass& operator=(SyclLoopInvariantMotionPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        bool changed = false;
        m_done.clear();
        for (int round = 0; round <= graph.node_size(); ++round) {
            if (!hoistOneFrame(graph)) {
                break;
            }
            changed = true;
        }
        return changed;
    }

private:
    static constexpr int k_default_parallel_iterations = 10;

    static bool modifiesFrame(const std::string& op)
    {
        return op == "Switch" || op == "RefSwitch" || op == "_SwitchN" || op == "Merge" ||
               op == "RefMerge" || op == "Enter" || op == "RefEnter" || op == "Exit" ||
               op == "RefExit" || op == "NextIteration" || op == "RefNextIteration" ||
               op == "LoopCond";
    }

    bool hoistOneFrame(tensorflow::GraphDef& graph)
    {
        m_scratch_new.clear();
        bool changed = false;
        {
            const SyclGraphIndex index{graph};
            const SyclFrameView frames{index};
            for (int position = 0; position < index.size() && !changed; ++position) {
                const tensorflow::NodeDef& node = index.node(position);
                if (!isInvariantEnter(node) || frames.frameOf(position) != frameName(node) ||
                    m_done.contains(frameName(node))) {
                    continue;
                }
                m_done.insert(frameName(node));
                changed = hoist(graph, index, frames, frameName(node));
            }
        }
        for (const auto& node: m_scratch_new) {
            *graph.add_node() = node;
        }
        return changed;
    }

    static std::string frameName(const tensorflow::NodeDef& node)
    {
        const auto name = node.attr().find("frame_name");
        return name == node.attr().end() ? std::string{} : name->second.s();
    }

    static bool isInvariantEnter(const tensorflow::NodeDef& node)
    {
        const auto constant = node.attr().find("is_constant");
        return (node.op() == "Enter" || node.op() == "RefEnter") && constant != node.attr().end() &&
               constant->second.b();
    }

    bool hoist(
        tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        const SyclFrameView& frames,
        const std::string& frame
    )
    {
        if (!frames.parentOf(frame).empty() || !m_context.getProperties().isInferred() ||
            loopConditionCount(index, frames, frame) != 1) {
            return false;
        }

        findInvariantNodes(index, frames, frame);
        pruneInvariantNodes(index, frames, frame);

        bool changed = false;
        for (const int node: m_scratch_invariant) {
            if (isHoistable(index, node) && rewire(graph, index, frames, frame, node)) {
                changed = true;
            }
        }
        return changed;
    }

    int loopConditionCount(
        const SyclGraphIndex& index,
        const SyclFrameView& frames,
        const std::string& frame
    ) const
    {
        int count = 0;
        for (int position = 0; position < index.size(); ++position) {
            if (index.node(position).op() == "LoopCond" && frames.frameOf(position) == frame) {
                ++count;
            }
        }
        return count;
    }

    // The constant Enters of the frame, then everything they feed that only reads invariant
    // nodes, Const nodes or constant Enters.
    void findInvariantNodes(
        const SyclGraphIndex& index,
        const SyclFrameView& frames,
        const std::string& frame
    )
    {
        m_scratch_invariant.clear();
        m_scratch_work.clear();
        for (int position = 0; position < index.size(); ++position) {
            const tensorflow::NodeDef& node = index.node(position);
            if (isInvariantEnter(node) && frames.frameOf(position) == frame &&
                frameName(node) == frame) {
                m_scratch_invariant.insert(position);
                m_scratch_work.push_back(position);
            }
        }
        while (!m_scratch_work.empty()) {
            const int current = m_scratch_work.back();
            m_scratch_work.pop_back();
            for (const auto& [reader, port]: index.consumersOf(current)) {
                if (m_scratch_invariant.contains(reader) || frames.frameOf(reader) != frame ||
                    !candidate(index, reader)) {
                    continue;
                }
                m_scratch_invariant.insert(reader);
                m_scratch_work.push_back(reader);
            }
        }
    }

    // A stateless op, defined by the host, none of the frame's structure ops, not a node the
    // caller must keep, whose data inputs are all invariant or Const.
    bool candidate(const SyclGraphIndex& index, int position)
    {
        const tensorflow::NodeDef& node = index.node(position);
        const auto stateful = m_context.getLookup().isStateful(node.op());
        if (modifiesFrame(node.op()) || node.op() == "Const" || !stateful || *stateful ||
            m_context.isPreserved(node.name())) {
            return false;
        }
        for (int slot = 0; slot < node.input_size(); ++slot) {
            if (node.input(slot).starts_with('^')) {
                continue;
            }
            const auto producer = index.dataInput(position, slot);
            if (!producer) {
                return false;
            }
            const bool constant = index.node(producer->first).op() == "Const";
            if (!constant && !m_scratch_invariant.contains(producer->first)) {
                return false;
            }
        }
        return true;
    }

    // Drops invariant nodes a variant node waits on through a control edge, and everything that
    // reads a dropped node.
    void pruneInvariantNodes(
        const SyclGraphIndex& index,
        const SyclFrameView& frames,
        const std::string& frame
    )
    {
        bool changed = true;
        while (changed) {
            changed = false;
            for (const int node: std::set<int>(m_scratch_invariant)) {
                if (!isHoistable(index, node)) {
                    continue;
                }
                if (hasVariantControlReader(index, node) || readsVariant(index, node)) {
                    m_scratch_invariant.erase(node);
                    changed = true;
                }
            }
        }
        static_cast<void>(frames);
        static_cast<void>(frame);
    }

    // Real computation, as opposed to the constant Enter nodes that start the invariant region.
    bool isHoistable(const SyclGraphIndex& index, int node) const
    {
        const tensorflow::NodeDef& source = index.node(node);
        return !isInvariantEnter(source) && source.op() != "Const";
    }

    bool hasVariantControlReader(const SyclGraphIndex& index, int node) const
    {
        const std::string& name = index.node(node).name();
        for (int reader = 0; reader < index.size(); ++reader) {
            if (m_scratch_invariant.contains(reader)) {
                continue;
            }
            for (const auto& input: index.node(reader).input()) {
                const auto edge = SyclGraphIndex::Edge::parse(input);
                if (edge.isControl() && edge.getName() == name) {
                    return true;
                }
            }
        }
        return false;
    }

    bool readsVariant(const SyclGraphIndex& index, int node) const
    {
        for (int slot = 0; slot < index.node(node).input_size(); ++slot) {
            const auto producer = index.dataInput(node, slot);
            if (producer && !m_scratch_invariant.contains(producer->first) &&
                index.node(producer->first).op() != "Const") {
                return true;
            }
        }
        return false;
    }

    // The node reads the pre-loop value of every constant Enter it used, loses control inputs on
    // the frame, and each variant reader of its outputs gets a new constant Enter.
    bool rewire(
        tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        const SyclFrameView& frames,
        const std::string& frame,
        int node
    )
    {
        tensorflow::NodeDef& target = *graph.mutable_node(node);
        const auto original = index.node(node);
        const bool feeds_variant = std::ranges::any_of(
            index.consumersOf(node),
            [this](const std::pair<int, int>& use)
            {
                return !m_scratch_invariant.contains(use.first);
            }
        );
        if (!feeds_variant) {
            return false;
        }

        target.clear_input();
        for (int slot = 0; slot < original.input_size(); ++slot) {
            const auto edge = SyclGraphIndex::Edge::parse(original.input(slot));
            const int producer = index.find(edge.getName());
            if (edge.isControl()) {
                if (producer >= 0 && frames.frameOf(producer) == frame) {
                    continue;
                }
                target.add_input(original.input(slot));
            } else if (
                producer >= 0 && isInvariantEnter(index.node(producer)) &&
                m_scratch_invariant.contains(producer)
            ) {
                target.add_input(index.node(producer).input(0));
            } else {
                target.add_input(original.input(slot));
            }
        }

        std::set<int> entered_ports;
        for (const auto& [reader, port]: index.consumersOf(node)) {
            if (m_scratch_invariant.contains(reader)) {
                continue;
            }
            const std::string name = enterFor(index, node, port, frame, entered_ports);
            if (name.empty()) {
                continue;
            }
            tensorflow::NodeDef& variant = *graph.mutable_node(reader);
            for (int slot = 0; slot < variant.input_size(); ++slot) {
                const auto edge = SyclGraphIndex::Edge::parse(variant.input(slot));
                if (!edge.isControl() && edge.getName() == original.name() &&
                    edge.getPort() == port) {
                    variant.set_input(slot, name);
                }
            }
        }
        return true;
    }

    // The constant Enter that carries one output of a hoisted node into the loop; created once.
    std::string enterFor(
        const SyclGraphIndex& index,
        int node,
        int port,
        const std::string& frame,
        std::set<int>& entered_ports
    )
    {
        const std::string name =
            "loop_invariant/" + index.node(node).name() + "_" + std::to_string(port);
        if (!entered_ports.insert(port).second) {
            return name;
        }
        const auto type = m_context.getProperties().output(index.node(node).name(), port);
        if (!type) {
            return {};
        }
        tensorflow::NodeDef enter;
        enter.set_name(name);
        enter.set_op("Enter");
        enter.add_input(index.node(node).name() + ":" + std::to_string(port));
        (*enter.mutable_attr())["T"].set_type(type->dtype());
        (*enter.mutable_attr())["frame_name"].set_s(frame);
        (*enter.mutable_attr())["is_constant"].set_b(true);
        (*enter.mutable_attr())["parallel_iterations"].set_i(parallelIterations(index, frame));
        m_scratch_new.push_back(std::move(enter));
        return name;
    }

    static int parallelIterations(const SyclGraphIndex& index, const std::string& frame)
    {
        for (int position = 0; position < index.size(); ++position) {
            const tensorflow::NodeDef& node = index.node(position);
            const auto count = node.attr().find("parallel_iterations");
            if (isInvariantEnter(node) && frameName(node) == frame && count != node.attr().end() &&
                count->second.i() > 0) {
                return static_cast<int>(count->second.i());
            }
        }
        return k_default_parallel_iterations;
    }

    SyclPassContext& m_context;
    std::set<std::string> m_done;
    std::set<int> m_scratch_invariant;
    std::vector<int> m_scratch_work;
    std::vector<tensorflow::NodeDef> m_scratch_new;
};

} // namespace aten_xpu
