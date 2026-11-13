module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:swapping_pass;

import std;
import :graph_index;
import :frame_view;
import :graph_memory;
import :pass_context;

export namespace aten_xpu {

// The swapping heuristic of TensorFlow's memory optimizer. When the estimated peak device memory
// exceeds the budget, tensors that are live at the peak but only read again after it move to the
// host: each late reader gets swap_out_<reader>_<slot> (a copy to host memory) and
// swap_in_<reader>_<slot> (the copy back), with swap_in held back by a control edge so it
// finishes just before the reader starts. Candidates are over 1 KiB, outside loops, and have
// time to come back at the 16 GB/s TensorFlow assumes for the link; they are ranked by
// (first late use - peak)^2 / uses^2 + (allocation - peak)^2 and taken until the saving covers
// the excess. A reader in a loop or next to control flow is never rewired. Up to 25 rounds.
class SyclSwappingPass
{
public:
    explicit SyclSwappingPass(SyclPassContext& context) noexcept :
        m_context{context},
        m_memory{context}
    {
    }

    SyclSwappingPass(const SyclSwappingPass&) = delete;
    SyclSwappingPass& operator=(const SyclSwappingPass&) = delete;
    SyclSwappingPass(SyclSwappingPass&&) = delete;
    SyclSwappingPass& operator=(SyclSwappingPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        if (m_context.getFetch().empty() || m_context.getMemoryLimitBytes() == 0 ||
            !m_context.getProperties().isInferred()) {
            return false;
        }
        bool changed = false;
        for (int round = 0; round < k_max_rounds; ++round) {
            if (!swapOnce(graph)) {
                break;
            }
            changed = true;
        }
        return changed;
    }

private:
    static constexpr int k_max_rounds = 25;
    static constexpr std::uint64_t k_small_tensor = 1'024;
    static constexpr std::uint64_t k_link_bandwidth = 16'000'000'000ULL;

    // A reader that can take the swapped tensor: after the peak, not in a loop, not control flow.
    class LateUse
    {
    public:
        int reader{0};
        int slot{0};
    };

    static bool isControlFlow(const std::string& op)
    {
        return op == "Switch" || op == "RefSwitch" || op == "_SwitchN" || op == "Merge" ||
               op == "RefMerge" || op == "Enter" || op == "RefEnter" || op == "Exit" ||
               op == "RefExit" || op == "NextIteration" || op == "RefNextIteration" ||
               op == "LoopCond" || op == "_CopyFromXpuToHost" || op == "_CopyFromHostToXpu";
    }

    bool swapOnce(tensorflow::GraphDef& graph)
    {
        m_scratch_new.clear();
        bool changed = false;
        {
            const SyclGraphIndex index{graph};
            const SyclFrameView frames{index};
            m_memory.analyze(graph, index);
            const std::uint64_t limit = m_context.getMemoryLimitBytes();
            if (m_memory.getPeakBytes() <= limit) {
                return false;
            }
            const std::uint64_t required = m_memory.getPeakBytes() - limit;
            rankCandidates(graph, index, frames);

            std::uint64_t saved = 0;
            for (const auto& candidate: m_scratch_ranked) {
                if (saved >= required) {
                    break;
                }
                saved += candidate.tensor.bytes;
                rewire(graph, index, candidate);
                changed = true;
            }
        }
        for (const auto& node: m_scratch_new) {
            *graph.add_node() = node;
        }
        return changed;
    }

    class Candidate
    {
    public:
        SyclGraphMemory::LiveTensor tensor;
        std::vector<LateUse> uses;
        double score{0.0};
    };

    void rankCandidates(
        const tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        const SyclFrameView& frames
    )
    {
        static_cast<void>(graph);
        m_scratch_ranked.clear();
        const std::int64_t peak = m_memory.getPeakTime();
        for (const auto& tensor: m_memory.getLiveAtPeak()) {
            const tensorflow::NodeDef& producer = index.node(tensor.node);
            if (tensor.persistent || tensor.bytes <= k_small_tensor ||
                !frames.frameOf(tensor.node).empty() || isControlFlow(producer.op()) ||
                m_context.isPreserved(producer.name())) {
                continue;
            }
            Candidate candidate;
            candidate.tensor = tensor;
            std::int64_t first_use = std::numeric_limits<std::int64_t>::max();
            for (const auto& [reader, port]: index.consumersOf(tensor.node)) {
                if (port != tensor.port || m_memory.startOf(reader) <= peak ||
                    !frames.frameOf(reader).empty() || isControlFlow(index.node(reader).op())) {
                    continue;
                }
                for (int slot = 0; slot < index.node(reader).input_size(); ++slot) {
                    const auto input = index.dataInput(reader, slot);
                    if (input && input->first == tensor.node && input->second == tensor.port) {
                        candidate.uses.push_back({reader, slot});
                    }
                }
                first_use = std::min(first_use, m_memory.startOf(reader));
            }
            const std::int64_t to_swap = m_memory.transferTime(tensor.bytes, k_link_bandwidth);
            if (candidate.uses.empty() || first_use - peak <= to_swap) {
                continue;
            }
            const auto idle = static_cast<double>(first_use - peak);
            const auto age = static_cast<double>(tensor.allocated - peak);
            const auto uses = static_cast<double>(candidate.uses.size());
            candidate.score = idle * idle / (uses * uses) + age * age;
            m_scratch_ranked.push_back(std::move(candidate));
        }
        std::ranges::sort(
            m_scratch_ranked,
            [](const Candidate& left, const Candidate& right)
            {
                return left.score > right.score;
            }
        );
    }

    void rewire(
        tensorflow::GraphDef& graph,
        const SyclGraphIndex& index,
        const Candidate& candidate
    )
    {
        const tensorflow::NodeDef& producer = index.node(candidate.tensor.node);
        const std::string tensor = producer.name() + ":" + std::to_string(candidate.tensor.port);
        for (const auto& use: candidate.uses) {
            tensorflow::NodeDef& reader = *graph.mutable_node(use.reader);
            const std::string suffix = reader.name() + "_" + std::to_string(use.slot);

            tensorflow::NodeDef out;
            out.set_name("swap_out_" + suffix);
            out.set_op("_CopyFromXpuToHost");
            out.add_input(tensor);
            (*out.mutable_attr())["T"].set_type(candidate.tensor.dtype);

            tensorflow::NodeDef in;
            in.set_name("swap_in_" + suffix);
            in.set_op("_CopyFromHostToXpu");
            in.add_input(out.name());
            (*in.mutable_attr())["T"].set_type(candidate.tensor.dtype);
            const int trigger = findSwapInTrigger(index, candidate, use);
            if (trigger >= 0) {
                in.add_input("^" + index.node(trigger).name());
            }
            reader.set_input(use.slot, in.name());
            m_scratch_new.push_back(std::move(out));
            m_scratch_new.push_back(std::move(in));
        }
    }

    // The latest node the reader already depends on that ends after the peak and early enough for
    // the copy back to finish before the reader starts; -1 when there is none.
    int findSwapInTrigger(
        const SyclGraphIndex& index,
        const Candidate& candidate,
        const LateUse& use
    ) const
    {
        const std::int64_t to_swap =
            m_memory.transferTime(candidate.tensor.bytes, k_link_bandwidth);
        const std::int64_t deadline = m_memory.startOf(use.reader) - to_swap;
        int best = -1;
        for (int slot = 0; slot < index.node(use.reader).input_size(); ++slot) {
            const auto input = index.dataInput(use.reader, slot);
            if (!input || input->first == candidate.tensor.node) {
                continue;
            }
            const std::int64_t end = m_memory.endOf(input->first);
            if (end > m_memory.getPeakTime() && end <= deadline &&
                (best < 0 || end > m_memory.endOf(best))) {
                best = input->first;
            }
        }
        return best;
    }

    SyclPassContext& m_context;
    SyclGraphMemory m_memory;
    std::vector<Candidate> m_scratch_ranked;
    std::vector<tensorflow::NodeDef> m_scratch_new;
};

} // namespace aten_xpu
