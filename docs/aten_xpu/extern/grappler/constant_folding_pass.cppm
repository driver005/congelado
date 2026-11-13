module;

#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:constant_folding_pass;

import std;
import :graph_index;
import :const_tensor;

export namespace aten_xpu {

// Evaluates float32 elementwise ops whose inputs are all Const nodes and replaces the op by a
// Const of the same name. Inputs must share a shape or be scalars, there must be no control
// edges, and the result stays under a size cap.
class SyclConstantFoldingPass
{
public:
    SyclConstantFoldingPass() noexcept = default;

    SyclConstantFoldingPass(const SyclConstantFoldingPass&) = delete;
    SyclConstantFoldingPass& operator=(const SyclConstantFoldingPass&) = delete;
    SyclConstantFoldingPass(SyclConstantFoldingPass&&) = delete;
    SyclConstantFoldingPass& operator=(SyclConstantFoldingPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        const SyclGraphIndex index{graph};
        bool changed = false;
        for (const int position: index.getTopologicalOrder()) {
            // The node is rewritten in place, so later nodes see the Const it became.
            tensorflow::NodeDef& node = *graph.mutable_node(position);
            if (node.op() == "Const" || index.hasControlEdges(position) || !isFloat(node)) {
                continue;
            }
            const auto result = fold(graph, index, position);
            if (result) {
                result->writeTo(node);
                changed = true;
            }
        }
        return changed;
    }

private:
    static constexpr std::size_t k_max_elements = std::size_t{1} << 20;

    static bool isFloat(const tensorflow::NodeDef& node)
    {
        const auto type = node.attr().find("T");
        return type != node.attr().end() && type->second.type() == tensorflow::DT_FLOAT;
    }

    std::optional<SyclConstTensor> constantInput(
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

    std::optional<SyclConstTensor>
    fold(const tensorflow::GraphDef& graph, const SyclGraphIndex& index, int position) const
    {
        const std::string& op = index.node(position).op();
        const int inputs = index.node(position).input_size();
        if (inputs == 1) {
            const auto operand = constantInput(graph, index, position, 0);
            return operand ? unary(op, *operand) : std::nullopt;
        }
        if (inputs == 2) {
            const auto left = constantInput(graph, index, position, 0);
            const auto right = constantInput(graph, index, position, 1);
            return left && right ? binary(op, *left, *right) : std::nullopt;
        }
        return std::nullopt;
    }

    std::optional<SyclConstTensor>
    unary(const std::string& op, const SyclConstTensor& operand) const
    {
        float (*function)(float) = nullptr;
        if (op == "Identity") {
            function = [](float value)
            {
                return value;
            };
        } else if (op == "Neg") {
            function = [](float value)
            {
                return -value;
            };
        } else if (op == "Abs") {
            function = [](float value)
            {
                return std::fabs(value);
            };
        } else if (op == "Square") {
            function = [](float value)
            {
                return value * value;
            };
        } else if (op == "Sqrt") {
            function = [](float value)
            {
                return std::sqrt(value);
            };
        } else if (op == "Rsqrt") {
            function = [](float value)
            {
                return 1.0F / std::sqrt(value);
            };
        } else if (op == "Reciprocal") {
            function = [](float value)
            {
                return 1.0F / value;
            };
        } else if (op == "Exp") {
            function = [](float value)
            {
                return std::exp(value);
            };
        } else if (op == "Log") {
            function = [](float value)
            {
                return std::log(value);
            };
        } else if (op == "Relu") {
            function = [](float value)
            {
                return std::max(value, 0.0F);
            };
        } else if (op == "Tanh") {
            function = [](float value)
            {
                return std::tanh(value);
            };
        } else if (op == "Sigmoid") {
            function = [](float value)
            {
                return 1.0F / (1.0F + std::exp(-value));
            };
        } else {
            return std::nullopt;
        }
        if (operand.getElementCount() > k_max_elements) {
            return std::nullopt;
        }
        std::vector<float> values(operand.getValues());
        for (float& value: values) {
            value = function(value);
        }
        return SyclConstTensor{operand.getDims(), std::move(values)};
    }

    std::optional<SyclConstTensor>
    binary(const std::string& op, const SyclConstTensor& left, const SyclConstTensor& right) const
    {
        float (*function)(float, float) = nullptr;
        if (op == "Add" || op == "AddV2") {
            function = [](float first, float second)
            {
                return first + second;
            };
        } else if (op == "Sub") {
            function = [](float first, float second)
            {
                return first - second;
            };
        } else if (op == "Mul") {
            function = [](float first, float second)
            {
                return first * second;
            };
        } else if (op == "RealDiv") {
            function = [](float first, float second)
            {
                return first / second;
            };
        } else if (op == "Maximum") {
            function = [](float first, float second)
            {
                return std::max(first, second);
            };
        } else if (op == "Minimum") {
            function = [](float first, float second)
            {
                return std::min(first, second);
            };
        } else {
            return std::nullopt;
        }

        const bool same = left.getDims() == right.getDims();
        if (!same && !left.isScalar() && !right.isScalar()) {
            return std::nullopt;
        }
        const SyclConstTensor& shape = left.isScalar() && !right.isScalar() ? right : left;
        if (shape.getElementCount() > k_max_elements) {
            return std::nullopt;
        }

        std::vector<float> values(shape.getElementCount());
        for (std::size_t index = 0; index < values.size(); ++index) {
            const float first =
                left.isScalar() ? left.getValues().front() : left.getValues()[index];
            const float second =
                right.isScalar() ? right.getValues().front() : right.getValues()[index];
            values[index] = function(first, second);
        }
        return SyclConstTensor{shape.getDims(), std::move(values)};
    }
};

} // namespace aten_xpu
