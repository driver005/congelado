module;

#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"

export module aten_xpu_extern_grappler:graph_index;

import std;

export namespace aten_xpu {

// Name lookup, data fanout, control edges and a topological order over a GraphDef.
class SyclGraphIndex
{
public:
    // One node input: "name", "name:port" or "^name" (control).
    class Edge
    {
    public:
        static Edge parse(std::string_view text)
        {
            Edge edge;
            if (text.starts_with('^')) {
                edge.m_control = true;
                text.remove_prefix(1);
            }
            const auto colon = text.rfind(':');
            if (colon != std::string_view::npos) {
                int port = 0;
                const auto digits = text.substr(colon + 1);
                const auto result =
                    std::from_chars(digits.data(), digits.data() + digits.size(), port);
                if (result.ec == std::errc{} && result.ptr == digits.data() + digits.size()) {
                    edge.m_port = port;
                    text = text.substr(0, colon);
                }
            }
            edge.m_name = text;
            return edge;
        }

        std::string_view getName() const noexcept
        {
            return m_name;
        }

        int getPort() const noexcept
        {
            return m_port;
        }

        bool isControl() const noexcept
        {
            return m_control;
        }

    private:
        std::string_view m_name;
        int m_port{0};
        bool m_control{false};
    };

    explicit SyclGraphIndex(const tensorflow::GraphDef& graph) :
        m_graph{graph}
    {
        const int count = graph.node_size();
        m_by_name.reserve(static_cast<std::size_t>(count));
        for (int index = 0; index < count; ++index) {
            m_by_name.emplace(graph.node(index).name(), index);
        }

        m_consumers.assign(static_cast<std::size_t>(count), {});
        m_control_consumers.assign(static_cast<std::size_t>(count), 0);
        m_pending_inputs.assign(static_cast<std::size_t>(count), 0);
        for (int index = 0; index < count; ++index) {
            for (const auto& input: graph.node(index).input()) {
                const auto edge = Edge::parse(input);
                const int producer = find(edge.getName());
                if (producer < 0) {
                    continue;
                }
                ++m_pending_inputs[static_cast<std::size_t>(index)];
                if (edge.isControl()) {
                    ++m_control_consumers[static_cast<std::size_t>(producer)];
                } else {
                    m_consumers[static_cast<std::size_t>(producer)].emplace_back(
                        index,
                        edge.getPort()
                    );
                }
            }
        }
        buildOrder();
    }

    SyclGraphIndex(const SyclGraphIndex&) = delete;
    SyclGraphIndex& operator=(const SyclGraphIndex&) = delete;
    SyclGraphIndex(SyclGraphIndex&&) = delete;
    SyclGraphIndex& operator=(SyclGraphIndex&&) = delete;

    int find(std::string_view name) const
    {
        const auto found = m_by_name.find(std::string{name});
        return found == m_by_name.end() ? -1 : found->second;
    }

    int size() const noexcept
    {
        return m_graph.node_size();
    }

    const tensorflow::NodeDef& node(int index) const
    {
        return m_graph.node(index);
    }

    // The number of data edges that read the given output port of a node.
    int dataFanout(int index, int port) const
    {
        const auto& consumers = m_consumers[static_cast<std::size_t>(index)];
        return static_cast<int>(std::ranges::count_if(
            consumers,
            [port](const std::pair<int, int>& use)
            {
                return use.second == port;
            }
        ));
    }

    // Every data edge that leaves a node: the reader and the output port it reads.
    const std::vector<std::pair<int, int>>& consumersOf(int index) const
    {
        return m_consumers[static_cast<std::size_t>(index)];
    }

    // The node that reads the given output port, when exactly one data edge does.
    std::optional<int> soleConsumer(int index, int port) const
    {
        std::optional<int> consumer;
        for (const auto& [reader, used_port]: m_consumers[static_cast<std::size_t>(index)]) {
            if (used_port != port) {
                continue;
            }
            if (consumer) {
                return std::nullopt;
            }
            consumer = reader;
        }
        return consumer;
    }

    bool hasControlEdges(int index) const
    {
        if (m_control_consumers[static_cast<std::size_t>(index)] > 0) {
            return true;
        }
        return std::ranges::any_of(
            node(index).input(),
            [](const std::string& input)
            {
                return input.starts_with('^');
            }
        );
    }

    // The data input at a position, as the producer node index and the port it reads.
    std::optional<std::pair<int, int>> dataInput(int index, int position) const
    {
        if (position < 0 || position >= node(index).input_size()) {
            return std::nullopt;
        }
        const auto edge = Edge::parse(node(index).input(position));
        const int producer = find(edge.getName());
        if (edge.isControl() || producer < 0) {
            return std::nullopt;
        }
        return std::pair<int, int>{producer, edge.getPort()};
    }

    const std::vector<int>& getTopologicalOrder() const noexcept
    {
        return m_order;
    }

private:
    // Kahn's algorithm; nodes on a cycle are appended last so every node appears once.
    void buildOrder()
    {
        const auto count = static_cast<std::size_t>(size());
        m_order.clear();
        m_order.reserve(count);
        m_scratch_placed.assign(count, false);
        std::deque<int> ready;
        for (std::size_t index = 0; index < count; ++index) {
            if (m_pending_inputs[index] == 0) {
                ready.push_back(static_cast<int>(index));
            }
        }
        while (!ready.empty()) {
            const int index = ready.front();
            ready.pop_front();
            m_scratch_placed[static_cast<std::size_t>(index)] = true;
            m_order.push_back(index);
            for (const auto& [reader, port]: m_consumers[static_cast<std::size_t>(index)]) {
                if (--m_pending_inputs[static_cast<std::size_t>(reader)] == 0) {
                    ready.push_back(reader);
                }
            }
            releaseControlReaders(index, ready);
        }
        for (std::size_t index = 0; index < count; ++index) {
            if (!m_scratch_placed[index]) {
                m_order.push_back(static_cast<int>(index));
            }
        }
    }

    // Control edges are not in m_consumers, so walk the readers' inputs for them.
    void releaseControlReaders(int producer, std::deque<int>& ready)
    {
        if (m_control_consumers[static_cast<std::size_t>(producer)] == 0) {
            return;
        }
        const std::string& name = node(producer).name();
        for (int reader = 0; reader < size(); ++reader) {
            for (const auto& input: node(reader).input()) {
                const auto edge = Edge::parse(input);
                if (edge.isControl() && edge.getName() == name &&
                    --m_pending_inputs[static_cast<std::size_t>(reader)] == 0) {
                    ready.push_back(reader);
                }
            }
        }
    }

    const tensorflow::GraphDef& m_graph;
    std::unordered_map<std::string, int> m_by_name;
    std::vector<std::vector<std::pair<int, int>>> m_consumers;
    std::vector<int> m_control_consumers;
    std::vector<int> m_pending_inputs;
    std::vector<int> m_order;
    std::vector<bool> m_scratch_placed;
};

} // namespace aten_xpu
