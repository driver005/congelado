module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/tensor.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:pin_to_host_pass;

import std;
import :graph_index;
import :pass_context;

export namespace aten_xpu {

// Pins small integer shape computations to the host, as TensorFlow's pin to host optimizer does:
// the data is tiny, so a device round trip costs more than the work. Shape, Size and Rank start
// the chain; an int32 op joins it when every input is already pinned or a small int32 Const, and
// such a Const is pinned when every reader is. Only unplaced nodes and nodes on an XPU move.
class SyclPinToHostPass
{
public:
    explicit SyclPinToHostPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclPinToHostPass(const SyclPinToHostPass&) = delete;
    SyclPinToHostPass& operator=(const SyclPinToHostPass&) = delete;
    SyclPinToHostPass(SyclPinToHostPass&&) = delete;
    SyclPinToHostPass& operator=(SyclPinToHostPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        const SyclGraphIndex index{graph};
        const auto count = static_cast<std::size_t>(index.size());
        m_scratch_pinned.assign(count, false);

        for (const int position: index.getTopologicalOrder()) {
            if (isShapeSource(index.node(position)) || joinsChain(index, position)) {
                m_scratch_pinned[static_cast<std::size_t>(position)] = true;
            }
        }
        // A small Const goes along only when every reader is pinned.
        for (int position = 0; position < index.size(); ++position) {
            if (isSmallInt32Const(index.node(position)) && readersPinned(index, position)) {
                m_scratch_pinned[static_cast<std::size_t>(position)] = true;
            }
        }

        bool changed = false;
        for (int position = 0; position < index.size(); ++position) {
            tensorflow::NodeDef& node = *graph.mutable_node(position);
            if (m_scratch_pinned[static_cast<std::size_t>(position)] && movable(node)) {
                node.set_device("/device:CPU:0");
                changed = true;
            }
        }
        return changed;
    }

private:
    static constexpr int k_max_const_elements = 64;

    static bool isShapeSource(const tensorflow::NodeDef& node)
    {
        return node.op() == "Shape" || node.op() == "Size" || node.op() == "Rank";
    }

    static bool isSmallInt32Const(const tensorflow::NodeDef& node)
    {
        const auto type = node.attr().find("dtype");
        const auto value = node.attr().find("value");
        if (node.op() != "Const" || type == node.attr().end() || value == node.attr().end() ||
            type->second.type() != tensorflow::DT_INT32) {
            return false;
        }
        std::int64_t count = 1;
        for (const auto& dimension: value->second.tensor().tensor_shape().dim()) {
            count *= dimension.size();
        }
        return count >= 0 && count <= k_max_const_elements;
    }

    static bool isHostableInt32Op(const tensorflow::NodeDef& node)
    {
        static const std::set<std::string> ops{
            "Identity",
            "Add",
            "AddV2",
            "Sub",
            "Mul",
            "Maximum",
            "Minimum",
            "Pack",
            "Unpack",
            "ConcatV2",
            "Slice",
            "StridedSlice",
            "Reshape"
        };
        const auto type = node.attr().find("T");
        return ops.contains(node.op()) && type != node.attr().end() &&
               type->second.type() == tensorflow::DT_INT32;
    }

    bool joinsChain(const SyclGraphIndex& index, int position) const
    {
        const tensorflow::NodeDef& node = index.node(position);
        if (!isHostableInt32Op(node) || node.input_size() == 0) {
            return false;
        }
        bool any_pinned = false;
        for (int slot = 0; slot < node.input_size(); ++slot) {
            const auto input = index.dataInput(position, slot);
            if (!input) {
                if (node.input(slot).starts_with('^')) {
                    continue;
                }
                return false;
            }
            const bool pinned = m_scratch_pinned[static_cast<std::size_t>(input->first)];
            if (!pinned && !isSmallInt32Const(index.node(input->first))) {
                return false;
            }
            any_pinned = any_pinned || pinned;
        }
        return any_pinned;
    }

    bool readersPinned(const SyclGraphIndex& index, int position) const
    {
        bool read = false;
        for (int reader = 0; reader < index.size(); ++reader) {
            for (int slot = 0; slot < index.node(reader).input_size(); ++slot) {
                const auto input = index.dataInput(reader, slot);
                if (input && input->first == position) {
                    read = true;
                    if (!m_scratch_pinned[static_cast<std::size_t>(reader)]) {
                        return false;
                    }
                }
            }
        }
        return read;
    }

    bool movable(const tensorflow::NodeDef& node) const
    {
        if (m_context.isPreserved(node.name())) {
            return false;
        }
        std::string device = node.device();
        std::ranges::transform(
            device,
            device.begin(),
            [](unsigned char letter)
            {
                return static_cast<char>(std::toupper(letter));
            }
        );
        return device.empty() || device.contains("XPU");
    }

    SyclPassContext& m_context;
    std::vector<bool> m_scratch_pinned;
};

} // namespace aten_xpu
