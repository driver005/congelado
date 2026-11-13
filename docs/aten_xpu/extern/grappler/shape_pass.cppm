module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/tensor_properties.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:shape_pass;

import std;
import :pass_context;

export namespace aten_xpu {

// Replaces Shape, Size and Rank nodes by Const nodes when the host's static shape inference knows
// the input shape completely, so shape arithmetic is folded before the device sees it. The
// replacement keeps the node's name; a node with control inputs is left alone.
class SyclShapePass
{
public:
    explicit SyclShapePass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclShapePass(const SyclShapePass&) = delete;
    SyclShapePass& operator=(const SyclShapePass&) = delete;
    SyclShapePass(SyclShapePass&&) = delete;
    SyclShapePass& operator=(SyclShapePass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        if (!m_context.getProperties().isInferred()) {
            return false;
        }
        bool changed = false;
        for (auto& node: *graph.mutable_node()) {
            const bool shape_op =
                node.op() == "Shape" || node.op() == "Size" || node.op() == "Rank";
            if (!shape_op || node.input_size() != 1 || node.input(0).starts_with('^')) {
                continue;
            }
            const auto properties = m_context.getProperties().input(node.name(), 0);
            if (!properties || properties->shape().unknown_rank() || !fullyKnown(*properties)) {
                continue;
            }
            changed |= replace(node, *properties);
        }
        return changed;
    }

private:
    static bool fullyKnown(const tensorflow::TensorProperties& properties)
    {
        return std::ranges::all_of(
            properties.shape().dim(),
            [](const tensorflow::TensorShapeProto::Dim& dimension)
            {
                return dimension.size() >= 0;
            }
        );
    }

    // Writes the node as a Const of the requested integer type; false when the value would not fit.
    bool replace(tensorflow::NodeDef& node, const tensorflow::TensorProperties& properties)
    {
        const auto out_type = node.attr().find(node.op() == "Rank" ? "" : "out_type");
        const auto type =
            out_type == node.attr().end() ? tensorflow::DT_INT32 : out_type->second.type();
        if (type != tensorflow::DT_INT32 && type != tensorflow::DT_INT64) {
            return false;
        }

        m_scratch_values.clear();
        std::int64_t count = 1;
        for (const auto& dimension: properties.shape().dim()) {
            count *= dimension.size();
            if (node.op() == "Shape") {
                m_scratch_values.push_back(dimension.size());
            }
        }
        if (node.op() == "Size") {
            m_scratch_values.push_back(count);
        } else if (node.op() == "Rank") {
            m_scratch_values.push_back(properties.shape().dim_size());
        }
        const bool fits = std::ranges::all_of(
            m_scratch_values,
            [type](std::int64_t value)
            {
                return type == tensorflow::DT_INT64 ||
                       value <= std::numeric_limits<std::int32_t>::max();
            }
        );
        if (!fits) {
            return false;
        }

        node.clear_input();
        node.mutable_attr()->clear();
        (*node.mutable_attr())["dtype"].set_type(type);
        auto* tensor = (*node.mutable_attr())["value"].mutable_tensor();
        tensor->set_dtype(type);
        if (node.op() == "Shape") {
            tensor->mutable_tensor_shape()->add_dim()->set_size(
                static_cast<std::int64_t>(m_scratch_values.size())
            );
        }
        for (const auto value: m_scratch_values) {
            if (type == tensorflow::DT_INT64) {
                tensor->add_int64_val(value);
            } else {
                tensor->add_int_val(static_cast<std::int32_t>(value));
            }
        }
        node.set_op("Const");
        return true;
    }

    SyclPassContext& m_context;
    std::vector<std::int64_t> m_scratch_values;
};

} // namespace aten_xpu
