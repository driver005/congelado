module;

#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"

export module aten_xpu_extern_grappler:pruning_pass;

import std;
import :graph_index;
import :pass_context;

export namespace aten_xpu {

// Drops the nodes nothing reads: those the fetch and preserved nodes do not depend on, unless the
// host defines their op as stateful. An op the host does not define is kept, since it may have
// side effects. Without fetch nodes there is nothing to prune toward, so the graph stays as is.
class SyclPruningPass
{
public:
    explicit SyclPruningPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclPruningPass(const SyclPruningPass&) = delete;
    SyclPruningPass& operator=(const SyclPruningPass&) = delete;
    SyclPruningPass(SyclPruningPass&&) = delete;
    SyclPruningPass& operator=(SyclPruningPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        if (m_context.getFetch().empty()) {
            return false;
        }

        const SyclGraphIndex index{graph};
        const auto count = static_cast<std::size_t>(index.size());
        m_scratch_live.assign(count, false);
        m_scratch_pending.clear();
        for (int position = 0; position < index.size(); ++position) {
            if (isRoot(index.node(position))) {
                m_scratch_live[static_cast<std::size_t>(position)] = true;
                m_scratch_pending.push_back(position);
            }
        }
        while (!m_scratch_pending.empty()) {
            const int current = m_scratch_pending.back();
            m_scratch_pending.pop_back();
            for (const auto& input: index.node(current).input()) {
                const int producer = index.find(SyclGraphIndex::Edge::parse(input).getName());
                if (producer >= 0 && !m_scratch_live[static_cast<std::size_t>(producer)]) {
                    m_scratch_live[static_cast<std::size_t>(producer)] = true;
                    m_scratch_pending.push_back(producer);
                }
            }
        }

        tensorflow::GraphDef kept;
        *kept.mutable_versions() = graph.versions();
        *kept.mutable_library() = graph.library();
        bool changed = false;
        for (int position = 0; position < index.size(); ++position) {
            if (m_scratch_live[static_cast<std::size_t>(position)]) {
                *kept.add_node() = index.node(position);
            } else {
                changed = true;
            }
        }
        if (changed) {
            graph.Swap(&kept);
        }
        return changed;
    }

private:
    bool isRoot(const tensorflow::NodeDef& node)
    {
        if (m_context.isPreserved(node.name())) {
            return true;
        }
        const auto stateful = m_context.getLookup().isStateful(node.op());
        return !stateful || *stateful;
    }

    SyclPassContext& m_context;
    std::vector<bool> m_scratch_live;
    std::vector<int> m_scratch_pending;
};

} // namespace aten_xpu
