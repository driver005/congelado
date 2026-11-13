module;

#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"

export module aten_xpu_extern_grappler:arithmetic_pass;

import std;
import :graph_index;
import :graph_editor;
import :const_tensor;
import :pass_context;

export namespace aten_xpu {

// Removes arithmetic that leaves a tensor unchanged: x * 1, x + 0, x - 0, x / 1 against a scalar
// float Const (a scalar cannot change the shape), Neg(Neg(x)) and Reciprocal(Reciprocal(x)).
// Reads of the removed node go to the operand; preserved nodes stay.
class SyclArithmeticPass
{
public:
    explicit SyclArithmeticPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclArithmeticPass(const SyclArithmeticPass&) = delete;
    SyclArithmeticPass& operator=(const SyclArithmeticPass&) = delete;
    SyclArithmeticPass(SyclArithmeticPass&&) = delete;
    SyclArithmeticPass& operator=(SyclArithmeticPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        SyclGraphEditor editor{graph};
        {
            const SyclGraphIndex index{graph};
            for (const int position: index.getTopologicalOrder()) {
                const tensorflow::NodeDef& node = index.node(position);
                if (m_context.isPreserved(node.name()) || index.hasControlEdges(position)) {
                    continue;
                }
                if (const auto operand = identityOperand(graph, index, position)) {
                    editor.bypass(node.name(), *operand);
                }
            }
        }
        return editor.apply();
    }

private:
    // The input the node simply passes through, when it is a no-op.
    std::optional<std::string> identityOperand(
        const tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        int position
    ) const
    {
        const tensorflow::NodeDef& node = index.node(position);
        const std::string& op = node.op();
        if (node.input_size() == 2) {
            const bool commutative = op == "Mul" || op == "Add" || op == "AddV2";
            const bool one_sided = op == "Sub" || op == "RealDiv";
            if (!commutative && !one_sided) {
                return std::nullopt;
            }
            const float neutral = (op == "Mul" || op == "RealDiv") ? 1.0F : 0.0F;
            if (isNeutral(graph, index, position, 1, neutral)) {
                return node.input(0);
            }
            if (commutative && isNeutral(graph, index, position, 0, neutral)) {
                return node.input(1);
            }
            return std::nullopt;
        }
        if (node.input_size() == 1 && (op == "Neg" || op == "Reciprocal")) {
            const auto input = index.dataInput(position, 0);
            if (!input || input->second != 0 || index.node(input->first).op() != op ||
                index.node(input->first).input_size() != 1) {
                return std::nullopt;
            }
            return index.node(input->first).input(0);
        }
        return std::nullopt;
    }

    bool isNeutral(
        const tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        int position,
        int slot,
        float neutral
    ) const
    {
        const auto input = index.dataInput(position, slot);
        if (!input || input->second != 0) {
            return false;
        }
        const auto constant = SyclConstTensor::fromNode(graph.node(input->first));
        return constant && constant->isScalarOf(neutral);
    }

    SyclPassContext& m_context;
};

} // namespace aten_xpu
