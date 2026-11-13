module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/tensor.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:const_tensor;

import std;

export namespace aten_xpu {

// A float32 Const node read into memory, so the graph passes can fold and rewrite constants.
class SyclConstTensor
{
public:
    SyclConstTensor() = default;

    SyclConstTensor(std::vector<std::int64_t> dims, std::vector<float> values) :
        m_dims{std::move(dims)},
        m_values{std::move(values)}
    {
    }

    // The tensor of a float Const node, or nothing for any other node or encoding.
    static std::optional<SyclConstTensor> fromNode(const tensorflow::NodeDef& node)
    {
        if (node.op() != "Const") {
            return std::nullopt;
        }
        const auto type = node.attr().find("dtype");
        const auto value = node.attr().find("value");
        if (type == node.attr().end() || value == node.attr().end() ||
            type->second.type() != tensorflow::DT_FLOAT) {
            return std::nullopt;
        }

        const auto& proto = value->second.tensor();
        if (proto.tensor_shape().unknown_rank()) {
            return std::nullopt;
        }
        SyclConstTensor tensor;
        std::int64_t count = 1;
        for (const auto& dimension: proto.tensor_shape().dim()) {
            if (dimension.size() < 0) {
                return std::nullopt;
            }
            tensor.m_dims.push_back(dimension.size());
            count *= dimension.size();
        }

        const auto size = static_cast<std::size_t>(count);
        tensor.m_values.resize(size);
        if (!proto.tensor_content().empty()) {
            if (proto.tensor_content().size() != size * sizeof(float)) {
                return std::nullopt;
            }
            std::memcpy(
                tensor.m_values.data(),
                proto.tensor_content().data(),
                size * sizeof(float)
            );
        } else if (static_cast<std::size_t>(proto.float_val_size()) == size) {
            std::ranges::copy(proto.float_val(), tensor.m_values.begin());
        } else if (proto.float_val_size() == 1) {
            std::ranges::fill(tensor.m_values, proto.float_val(0));
        } else if (size != 0) {
            return std::nullopt;
        }
        return tensor;
    }

    // Turns the node into a Const holding this tensor; inputs and other attrs are dropped.
    void writeTo(tensorflow::NodeDef& node) const
    {
        node.set_op("Const");
        node.clear_input();
        node.mutable_attr()->clear();
        (*node.mutable_attr())["dtype"].set_type(tensorflow::DT_FLOAT);
        auto* proto = (*node.mutable_attr())["value"].mutable_tensor();
        proto->set_dtype(tensorflow::DT_FLOAT);
        for (const auto size: m_dims) {
            proto->mutable_tensor_shape()->add_dim()->set_size(size);
        }
        proto->set_tensor_content(m_values.data(), m_values.size() * sizeof(float));
    }

    const std::vector<std::int64_t>& getDims() const noexcept
    {
        return m_dims;
    }

    const std::vector<float>& getValues() const noexcept
    {
        return m_values;
    }

    std::size_t getElementCount() const noexcept
    {
        return m_values.size();
    }

    bool isScalar() const noexcept
    {
        return m_dims.empty() && m_values.size() == 1;
    }

    bool isScalarOf(float value) const noexcept
    {
        return isScalar() && m_values.front() == value;
    }

private:
    std::vector<std::int64_t> m_dims;
    std::vector<float> m_values;
};

} // namespace aten_xpu
