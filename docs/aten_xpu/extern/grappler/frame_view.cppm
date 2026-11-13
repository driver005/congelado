module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"

export module aten_xpu_extern_grappler:frame_view;

import std;
import :graph_index;

export namespace aten_xpu {

// The loop frame each node runs in. An Enter starts a frame (its frame_name attr), an Exit leaves
// it for the parent frame (the frame of the Enter's input), and every other node runs in the frame
// of its data inputs. Nodes outside any loop have the empty frame name.
class SyclFrameView
{
public:
    explicit SyclFrameView(const SyclGraphIndex& index)
    {
        const auto count = static_cast<std::size_t>(index.size());
        m_frames.assign(count, std::string{});
        bool changed = true;
        for (std::size_t round = 0; changed && round <= count; ++round) {
            changed = false;
            for (int position = 0; position < index.size(); ++position) {
                changed |= update(index, position);
            }
        }
    }

    SyclFrameView(const SyclFrameView&) = delete;
    SyclFrameView& operator=(const SyclFrameView&) = delete;
    SyclFrameView(SyclFrameView&&) = delete;
    SyclFrameView& operator=(SyclFrameView&&) = delete;

    const std::string& frameOf(int position) const
    {
        return m_frames[static_cast<std::size_t>(position)];
    }

    // The frame a frame is nested in; empty for an outermost loop.
    std::string parentOf(const std::string& frame) const
    {
        const auto found = m_parents.find(frame);
        return found == m_parents.end() ? std::string{} : found->second;
    }

private:
    bool update(const SyclGraphIndex& index, int position)
    {
        const tensorflow::NodeDef& node = index.node(position);
        std::string frame;
        const auto input = index.dataInput(position, 0);
        if (node.op() == "Enter" || node.op() == "RefEnter") {
            const auto name = node.attr().find("frame_name");
            frame = name == node.attr().end() ? std::string{} : name->second.s();
            if (input) {
                m_parents[frame] = m_frames[static_cast<std::size_t>(input->first)];
            }
        } else if (node.op() == "Exit" || node.op() == "RefExit") {
            if (input) {
                frame = parentOf(m_frames[static_cast<std::size_t>(input->first)]);
            }
        } else {
            for (int slot = 0; slot < node.input_size() && frame.empty(); ++slot) {
                const auto producer = index.dataInput(position, slot);
                if (producer) {
                    frame = m_frames[static_cast<std::size_t>(producer->first)];
                }
            }
        }
        std::string& current = m_frames[static_cast<std::size_t>(position)];
        if (current == frame) {
            return false;
        }
        current = std::move(frame);
        return true;
    }

    std::vector<std::string> m_frames;
    std::map<std::string, std::string> m_parents;
};

} // namespace aten_xpu
