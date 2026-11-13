module;

#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"

export module aten_xpu_extern_grappler:graph_editor;

import std;
import :graph_index;

export namespace aten_xpu {

// Collects node removals and read redirects, then applies them to the graph in one sweep.
class SyclGraphEditor
{
public:
    explicit SyclGraphEditor(tensorflow::GraphDef& graph) noexcept :
        m_graph{graph}
    {
    }

    SyclGraphEditor(const SyclGraphEditor&) = delete;
    SyclGraphEditor& operator=(const SyclGraphEditor&) = delete;
    SyclGraphEditor(SyclGraphEditor&&) = delete;
    SyclGraphEditor& operator=(SyclGraphEditor&&) = delete;

    // Every read of the node's first output reads `replacement` ("node" or "node:port") instead,
    // control dependencies on the node move to the replacement's node, and the node is dropped.
    void bypass(const std::string& name, std::string replacement)
    {
        m_redirects[name] = std::move(replacement);
        m_removed.insert(name);
    }

    // Reads of one output port of a node read `replacement` instead; the node itself stays unless
    // it is also bypassed or removed.
    void redirect(const std::string& name, int port, std::string replacement)
    {
        m_port_redirects[{name, port}] = std::move(replacement);
    }

    void remove(const std::string& name)
    {
        m_removed.insert(name);
    }

    bool hasWork() const noexcept
    {
        return !m_removed.empty() || !m_port_redirects.empty();
    }

    // Returns true when the graph changed.
    bool apply()
    {
        if (!hasWork()) {
            return false;
        }

        tensorflow::GraphDef kept;
        *kept.mutable_versions() = m_graph.versions();
        *kept.mutable_library() = m_graph.library();
        for (const auto& node: m_graph.node()) {
            if (m_removed.contains(node.name())) {
                continue;
            }
            tensorflow::NodeDef* copy = kept.add_node();
            *copy = node;
            copy->clear_input();
            for (const auto& input: node.input()) {
                const std::string target = redirected(input);
                if (!dropped(target)) {
                    copy->add_input(target);
                }
            }
        }
        m_graph.Swap(&kept);
        return true;
    }

private:
    // A control input on a node that was removed without a replacement has nothing to wait for.
    bool dropped(const std::string& input) const
    {
        const auto edge = SyclGraphIndex::Edge::parse(input);
        return edge.isControl() && m_removed.contains(std::string{edge.getName()});
    }

    // Follows redirects until the read lands on a node that stays.
    std::string redirected(const std::string& input) const
    {
        std::string current = input;
        for (std::size_t step = 0; step <= m_redirects.size() + m_port_redirects.size(); ++step) {
            const auto edge = SyclGraphIndex::Edge::parse(current);
            if (!edge.isControl()) {
                const auto port =
                    m_port_redirects.find({std::string{edge.getName()}, edge.getPort()});
                if (port != m_port_redirects.end()) {
                    current = port->second;
                    continue;
                }
            }
            const auto found = m_redirects.find(std::string{edge.getName()});
            if (found == m_redirects.end() || (!edge.isControl() && edge.getPort() != 0)) {
                return current;
            }
            if (edge.isControl()) {
                const auto target = SyclGraphIndex::Edge::parse(found->second);
                current = "^" + std::string{target.getName()};
            } else {
                current = found->second;
            }
        }
        return current;
    }

    tensorflow::GraphDef& m_graph;
    std::map<std::string, std::string> m_redirects;
    std::map<std::pair<std::string, int>, std::string> m_port_redirects;
    std::set<std::string> m_removed;
};

} // namespace aten_xpu
