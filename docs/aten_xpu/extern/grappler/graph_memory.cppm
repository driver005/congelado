module;

#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:graph_memory;

import std;
import :graph_index;
import :pass_context;
import :tensor_bytes;

export namespace aten_xpu {

// A static estimate of device memory over one run of the graph, the plugin's stand-in for
// TensorFlow's GraphMemory (the plugin has no cost graph or virtual cluster). Nodes run in
// topological order; each takes the time to move its input and output bytes at the device's
// memory bandwidth (at least a microsecond). An output tensor is live from the end of its node
// until its last reader ends; constants, placeholders and variables are live throughout, and
// outputs on the host (the swap copies) take no device memory.
class SyclGraphMemory
{
public:
    class LiveTensor
    {
    public:
        int node{0};
        int port{0};
        std::uint64_t bytes{0};
        std::int64_t allocated{0};
        std::int64_t released{0};
        bool persistent{false};
        tensorflow::DataType dtype{tensorflow::DT_INVALID};
    };

    explicit SyclGraphMemory(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclGraphMemory(const SyclGraphMemory&) = delete;
    SyclGraphMemory& operator=(const SyclGraphMemory&) = delete;
    SyclGraphMemory(SyclGraphMemory&&) = delete;
    SyclGraphMemory& operator=(SyclGraphMemory&&) = delete;

    void analyze(const tensorflow::GraphDef& graph, const SyclGraphIndex& index)
    {
        m_tensors.clear();
        m_live_at_peak.clear();
        m_peak_bytes = 0;
        m_peak_time = 0;
        const auto count = static_cast<std::size_t>(index.size());
        m_start.assign(count, 0);
        m_end.assign(count, 0);
        m_scratch_sizes.assign(count, {});

        std::int64_t now = 0;
        for (const int position: index.getTopologicalOrder()) {
            collectSizes(index, position);
            m_start[static_cast<std::size_t>(position)] = now;
            now += duration(index, position);
            m_end[static_cast<std::size_t>(position)] = now;
        }
        m_total_time = now;
        buildTensors(graph, index);
        findPeak();
    }

    std::uint64_t getPeakBytes() const noexcept
    {
        return m_peak_bytes;
    }

    std::int64_t getPeakTime() const noexcept
    {
        return m_peak_time;
    }

    const std::vector<LiveTensor>& getLiveAtPeak() const noexcept
    {
        return m_live_at_peak;
    }

    std::int64_t startOf(int node) const
    {
        return m_start[static_cast<std::size_t>(node)];
    }

    std::int64_t endOf(int node) const
    {
        return m_end[static_cast<std::size_t>(node)];
    }

    // Nanoseconds to move the given bytes at the device's memory bandwidth; 0 when it is unknown.
    std::int64_t transferTime(std::uint64_t bytes, std::uint64_t bandwidth) const
    {
        return bandwidth == 0
                   ? 0
                   : static_cast<std::int64_t>(
                         static_cast<double>(bytes) * 1.0e9 / static_cast<double>(bandwidth)
                     );
    }

private:
    static constexpr std::int64_t k_minimum_duration = 1'000;

    static bool isPersistent(const std::string& op)
    {
        return op == "Const" || op == "Placeholder" || op == "PlaceholderWithDefault" ||
               op == "Variable" || op == "VariableV2" || op == "VarHandleOp";
    }

    static bool onHost(const std::string& op)
    {
        return op == "_CopyFromXpuToHost";
    }

    // The bytes and dtype of each output port the host's inference reports; nothing for host
    // outputs or nodes it does not know.
    void collectSizes(const SyclGraphIndex& index, int position)
    {
        auto& sizes = m_scratch_sizes[static_cast<std::size_t>(position)];
        const tensorflow::NodeDef& node = index.node(position);
        if (onHost(node.op())) {
            return;
        }
        const int outputs = m_context.getProperties().outputCount(node.name());
        for (int port = 0; port < outputs; ++port) {
            const auto properties = m_context.getProperties().output(node.name(), port);
            const auto bytes = properties ? m_bytes.of(*properties) : std::nullopt;
            sizes.emplace_back(
                bytes.value_or(0),
                properties ? properties->dtype() : tensorflow::DT_INVALID
            );
        }
    }

    std::int64_t duration(const SyclGraphIndex& index, int position) const
    {
        std::uint64_t moved = 0;
        for (const auto& size: m_scratch_sizes[static_cast<std::size_t>(position)]) {
            moved += size.first;
        }
        for (int slot = 0; slot < index.node(position).input_size(); ++slot) {
            const auto producer = index.dataInput(position, slot);
            if (producer) {
                const auto& sizes = m_scratch_sizes[static_cast<std::size_t>(producer->first)];
                if (static_cast<std::size_t>(producer->second) < sizes.size()) {
                    moved += sizes[static_cast<std::size_t>(producer->second)].first;
                }
            }
        }
        return std::max(k_minimum_duration, transferTime(moved, m_context.getDeviceBandwidth()));
    }

    void buildTensors(const tensorflow::GraphDef& graph, const SyclGraphIndex& index)
    {
        static_cast<void>(graph);
        for (int position = 0; position < index.size(); ++position) {
            const auto& sizes = m_scratch_sizes[static_cast<std::size_t>(position)];
            for (std::size_t port = 0; port < sizes.size(); ++port) {
                if (sizes[port].first == 0) {
                    continue;
                }
                LiveTensor tensor;
                tensor.node = position;
                tensor.port = static_cast<int>(port);
                tensor.bytes = sizes[port].first;
                tensor.dtype = sizes[port].second;
                tensor.persistent = isPersistent(index.node(position).op());
                tensor.allocated = tensor.persistent ? 0 : endOf(position);
                tensor.released = lastUse(index, position, tensor.port);
                m_tensors.push_back(tensor);
            }
        }
    }

    // When the last reader of the tensor ends; the end of the run for a tensor the caller keeps.
    std::int64_t lastUse(const SyclGraphIndex& index, int position, int port) const
    {
        if (isPersistent(index.node(position).op()) ||
            m_context.isPreserved(index.node(position).name())) {
            return m_total_time;
        }
        std::int64_t last = endOf(position);
        for (const auto& [reader, used_port]: index.consumersOf(position)) {
            if (used_port == port) {
                last = std::max(last, endOf(reader));
            }
        }
        return last;
    }

    void findPeak()
    {
        m_scratch_events.clear();
        for (const auto& tensor: m_tensors) {
            m_scratch_events.emplace_back(
                tensor.allocated,
                static_cast<std::int64_t>(tensor.bytes)
            );
            m_scratch_events.emplace_back(
                tensor.released,
                -static_cast<std::int64_t>(tensor.bytes)
            );
        }
        // Releases sort before allocations at the same instant.
        std::ranges::sort(m_scratch_events);
        std::int64_t usage = 0;
        std::int64_t peak = 0;
        for (const auto& [time, delta]: m_scratch_events) {
            usage += delta;
            if (usage > peak) {
                peak = usage;
                m_peak_time = time;
            }
        }
        m_peak_bytes = static_cast<std::uint64_t>(peak);
        for (const auto& tensor: m_tensors) {
            if (tensor.allocated <= m_peak_time && m_peak_time < tensor.released) {
                m_live_at_peak.push_back(tensor);
            }
        }
    }

    SyclPassContext& m_context;
    SyclTensorBytes m_bytes;
    std::vector<LiveTensor> m_tensors;
    std::vector<LiveTensor> m_live_at_peak;
    std::vector<std::int64_t> m_start;
    std::vector<std::int64_t> m_end;
    std::vector<std::vector<std::pair<std::uint64_t, tensorflow::DataType>>> m_scratch_sizes;
    std::vector<std::pair<std::int64_t, std::int64_t>> m_scratch_events;
    std::uint64_t m_peak_bytes{0};
    std::int64_t m_peak_time{0};
    std::int64_t m_total_time{0};
};

} // namespace aten_xpu
