module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"

export module aten_xpu_extern_grappler:stack_push_removal_pass;

import std;
import :graph_index;
import :pass_context;

export namespace aten_xpu {

// Removes stack pushes nobody can read back, as TensorFlow's loop optimizer does: when every pop of
// a stack has no readers (and the stack is not preserved), each push becomes an Identity of the
// pushed value with the stack handle as a control input. A stack with a used pop is left alone.
class SyclStackPushRemovalPass
{
public:
    explicit SyclStackPushRemovalPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclStackPushRemovalPass(const SyclStackPushRemovalPass&) = delete;
    SyclStackPushRemovalPass& operator=(const SyclStackPushRemovalPass&) = delete;
    SyclStackPushRemovalPass(SyclStackPushRemovalPass&&) = delete;
    SyclStackPushRemovalPass& operator=(SyclStackPushRemovalPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        m_scratch_pushes.clear();
        {
            const SyclGraphIndex index{graph};
            for (int position = 0; position < index.size(); ++position) {
                const std::string& op = index.node(position).op();
                if (op == "Stack" || op == "StackV2") {
                    collectPushes(index, position);
                }
            }
        }

        bool changed = false;
        for (const int push: m_scratch_pushes) {
            tensorflow::NodeDef& node = *graph.mutable_node(push);
            if (node.op() == "Identity" || node.input_size() < 2) {
                continue;
            }
            const std::string handle = node.input(0);
            const std::string value = node.input(1);
            node.set_op("Identity");
            node.clear_input();
            node.add_input(value);
            node.add_input("^" + handle.substr(0, handle.find(':')));
            node.mutable_attr()->erase("swap_memory");
            changed = true;
        }
        return changed;
    }

private:
    static bool isPush(const std::string& op)
    {
        return op == "StackPush" || op == "StackPushV2";
    }

    static bool isPop(const std::string& op)
    {
        return op == "StackPop" || op == "StackPopV2";
    }

    static bool passesHandle(const std::string& op)
    {
        return op == "Stack" || op == "StackV2" || op == "Enter" || op == "RefEnter" ||
               op == "Switch" || op == "RefSwitch" || op == "_SwitchN" || op == "Identity" ||
               op == "RefIdentity";
    }

    // Follows the handle through the ops that forward it. The pushes found are added to the list
    // only when every pop on the way is unread; a preserved stack or a used pop drops them all.
    void collectPushes(const SyclGraphIndex& index, int stack)
    {
        if (m_context.isPreserved(index.node(stack).name())) {
            return;
        }
        m_scratch_found.clear();
        m_scratch_visited.clear();
        m_scratch_work.assign(1, stack);
        m_scratch_visited.insert(stack);
        while (!m_scratch_work.empty()) {
            const int current = m_scratch_work.back();
            m_scratch_work.pop_back();
            for (const auto& [reader, port]: index.consumersOf(current)) {
                const std::string& op = index.node(reader).op();
                if (isPush(op)) {
                    if (index.dataInput(reader, 0) &&
                        index.dataInput(reader, 0)->first == current) {
                        m_scratch_found.push_back(reader);
                    }
                } else if (isPop(op)) {
                    if (!index.consumersOf(reader).empty() ||
                        m_context.isPreserved(index.node(reader).name())) {
                        return;
                    }
                } else if (passesHandle(op) && m_scratch_visited.insert(reader).second) {
                    m_scratch_work.push_back(reader);
                }
            }
        }
        m_scratch_pushes
            .insert(m_scratch_pushes.end(), m_scratch_found.begin(), m_scratch_found.end());
    }

    SyclPassContext& m_context;
    std::vector<int> m_scratch_pushes;
    std::vector<int> m_scratch_found;
    std::vector<int> m_scratch_work;
    std::set<int> m_scratch_visited;
};

} // namespace aten_xpu
