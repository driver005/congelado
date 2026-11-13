module;

#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"

export module aten_xpu_extern_grappler:debug_stripper_pass;

import std;
import :graph_index;
import :graph_editor;
import :pass_context;

export namespace aten_xpu {

// Strips debugging ops, as TensorFlow's debug stripper does: Assert and PrintV2 nodes are removed
// along with the control inputs that waited on them, Print passes its input through, and
// CheckNumerics becomes an Identity. Preserved nodes are not removed.
class SyclDebugStripperPass
{
public:
    explicit SyclDebugStripperPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclDebugStripperPass(const SyclDebugStripperPass&) = delete;
    SyclDebugStripperPass& operator=(const SyclDebugStripperPass&) = delete;
    SyclDebugStripperPass(SyclDebugStripperPass&&) = delete;
    SyclDebugStripperPass& operator=(SyclDebugStripperPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        bool changed = false;
        SyclGraphEditor editor{graph};
        for (auto& node: *graph.mutable_node()) {
            const std::string& op = node.op();
            if (op == "CheckNumerics") {
                // Keeps its name, so a preserved node converts as well.
                changed |= toIdentity(node);
            } else if (m_context.isPreserved(node.name())) {
                continue;
            } else if (op == "Assert" || op == "PrintV2") {
                editor.remove(node.name());
            } else if (op == "Print" && node.input_size() > 0 && !node.input(0).starts_with('^')) {
                editor.bypass(node.name(), node.input(0));
            }
        }
        return editor.apply() || changed;
    }

private:
    static bool toIdentity(tensorflow::NodeDef& node)
    {
        const auto type = node.attr().find("T");
        if (type == node.attr().end()) {
            return false;
        }
        const auto kept = type->second;
        node.set_op("Identity");
        node.mutable_attr()->clear();
        (*node.mutable_attr())["T"] = kept;
        return true;
    }

    SyclPassContext& m_context;
};

} // namespace aten_xpu
