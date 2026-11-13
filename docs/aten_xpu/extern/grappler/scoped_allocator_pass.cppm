module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "cc/proto/tensor_properties.pb.h"
#include "cc/proto/types.pb.h"

export module aten_xpu_extern_grappler:scoped_allocator_pass;

import std;
import :graph_index;
import :pass_context;
import :tensor_bytes;

export namespace aten_xpu {

// TensorFlow's scoped allocator optimizer: ops of the configured kinds (default CollectiveReduce)
// that sit in the same name scope, on the same device and dtype, and each read one tensor of a
// fully known shape, are fused into one op over a single buffer. A _ScopedAllocator makes the
// backing buffer, _ScopedAllocatorConcat packs the inputs into it, one copy of the op runs over
// the whole buffer, and _ScopedAllocatorSplit hands every original consumer its slice. The op's
// attrs come from the first group member. A group is dropped when a member has control edges, is
// preserved, reads a Const, Exit or _Arg node, reads another member, or already has a scope.
// The plugin's Split and Concat copy: the host's executor is what could hand producers the
// backing buffer directly, and it is told nothing here, so there is no allocation saving.
class SyclScopedAllocatorPass
{
public:
    explicit SyclScopedAllocatorPass(SyclPassContext& context) noexcept :
        m_context{context}
    {
    }

    SyclScopedAllocatorPass(const SyclScopedAllocatorPass&) = delete;
    SyclScopedAllocatorPass& operator=(const SyclScopedAllocatorPass&) = delete;
    SyclScopedAllocatorPass(SyclScopedAllocatorPass&&) = delete;
    SyclScopedAllocatorPass& operator=(SyclScopedAllocatorPass&&) = delete;

    bool run(tensorflow::GraphDef& graph)
    {
        if (m_context.getScopedAllocatorOps().empty() || !m_context.getProperties().isInferred()) {
            return false;
        }
        m_scratch_new.clear();
        m_scratch_removed.clear();
        m_scratch_redirects.clear();
        m_next_id = 1;
        {
            const SyclGraphIndex index{graph};
            collectGroups(graph, index);
            for (const auto& group: m_scratch_groups) {
                rewrite(graph, index, group);
            }
        }
        if (m_scratch_removed.empty()) {
            return false;
        }

        tensorflow::GraphDef output;
        *output.mutable_versions() = graph.versions();
        *output.mutable_library() = graph.library();
        for (const auto& node: graph.node()) {
            if (m_scratch_removed.contains(node.name())) {
                continue;
            }
            tensorflow::NodeDef* copy = output.add_node();
            *copy = node;
            for (int slot = 0; slot < copy->input_size(); ++slot) {
                const auto found = m_scratch_redirects.find(copy->input(slot));
                if (found != m_scratch_redirects.end()) {
                    copy->set_input(slot, found->second);
                }
            }
        }
        for (const auto& node: m_scratch_new) {
            *output.add_node() = node;
        }
        graph = std::move(output);
        return true;
    }

private:
    static constexpr std::uint64_t k_alignment = 64;

    class Group
    {
    public:
        std::vector<int> members;
    };

    bool eligible(const tensorflow::GraphDef& graph, const SyclGraphIndex& index, int position)
    {
        const tensorflow::NodeDef& node = index.node(position);
        if (!m_context.getScopedAllocatorOps().contains(node.op()) ||
            m_context.isPreserved(node.name()) || index.hasControlEdges(position) ||
            node.input_size() != 1 || !node.attr().contains("T") ||
            node.attr().contains("_scoped_allocator")) {
            return false;
        }
        const auto producer = index.dataInput(position, 0);
        if (!producer) {
            return false;
        }
        const std::string& source = graph.node(producer->first).op();
        const auto output = m_context.getProperties().output(node.name(), 0);
        const auto input =
            m_context.getProperties().output(graph.node(producer->first).name(), producer->second);
        const auto size = input ? m_bytes.elementSize(input->dtype()) : std::nullopt;
        return source != "Const" && source != "Exit" && source != "_Arg" && output && input &&
               !input->shape().unknown_rank() && m_bytes.of(*input) && size &&
               k_alignment % *size == 0 && m_context.getProperties().outputCount(node.name()) == 1;
    }

    // Groups share a device, op, dtype and name scope (the part of the name before the last '/').
    void collectGroups(const tensorflow::GraphDef& graph, const SyclGraphIndex& index)
    {
        m_scratch_groups.clear();
        std::map<std::string, Group> buckets;
        for (const int position: index.getTopologicalOrder()) {
            if (!eligible(graph, index, position)) {
                continue;
            }
            const tensorflow::NodeDef& node = index.node(position);
            const auto scope_end = node.name().rfind('/');
            const std::string scope =
                scope_end == std::string::npos ? std::string{} : node.name().substr(0, scope_end);
            const std::string key = node.device() + "|" + node.op() + "|" +
                                    std::to_string(static_cast<int>(node.attr().at("T").type())) +
                                    "|" + scope;
            buckets[key].members.push_back(position);
        }
        for (auto& [key, group]: buckets) {
            if (group.members.size() > 1 && independent(index, group)) {
                m_scratch_groups.push_back(std::move(group));
            }
        }
    }

    // No member reads another member, and no member reads a tensor that another member also reads.
    bool independent(const SyclGraphIndex& index, const Group& group) const
    {
        std::set<std::pair<int, int>> inputs;
        for (const int member: group.members) {
            const auto input = index.dataInput(member, 0);
            if (!input || std::ranges::contains(group.members, input->first) ||
                !inputs.insert(*input).second) {
                return false;
            }
        }
        return true;
    }

    void rewrite(const tensorflow::GraphDef& graph, const SyclGraphIndex& index, const Group& group)
    {
        const int id = m_next_id++;
        const std::string name = "scoped_allocator_" + std::to_string(id);
        const tensorflow::NodeDef& first = index.node(group.members.front());
        const auto dtype = first.attr().at("T").type();
        const int count = static_cast<int>(group.members.size());

        std::uint64_t total = 0;
        m_scratch_dims.clear();
        m_scratch_ranks.clear();
        for (const int member: group.members) {
            const auto input = index.dataInput(member, 0);
            const auto properties =
                m_context.getProperties().output(graph.node(input->first).name(), input->second);
            total += *m_bytes.of(*properties) / *m_bytes.elementSize(dtype);
            m_scratch_ranks.push_back(properties->shape().dim_size());
            for (const auto& dimension: properties->shape().dim()) {
                m_scratch_dims.push_back(dimension.size());
            }
        }

        tensorflow::NodeDef allocator;
        allocator.set_name(name);
        allocator.set_op("_ScopedAllocator");
        allocator.set_device(first.device());
        (*allocator.mutable_attr())["T"].set_type(dtype);
        (*allocator.mutable_attr())["total"].set_i(static_cast<std::int64_t>(total));
        (*allocator.mutable_attr())["sa_name"].set_s(name);
        (*allocator.mutable_attr())["id"].set_i(id);
        (*allocator.mutable_attr())["expected_call_count"].set_i(count);
        allocator.add_input(
            "^" + graph.node(index.dataInput(group.members.front(), 0)->first).name()
        );

        tensorflow::NodeDef concat;
        concat.set_name(name + "/concat");
        concat.set_op("_ScopedAllocatorConcat");
        concat.set_device(first.device());
        concat.add_input(name);
        for (const int member: group.members) {
            concat.add_input(index.node(member).input(0));
        }
        (*concat.mutable_attr())["T"].set_type(dtype);
        (*concat.mutable_attr())["total"].set_i(static_cast<std::int64_t>(total));
        (*concat.mutable_attr())["sa_name"].set_s(name);
        (*concat.mutable_attr())["id"].set_i(id);
        (*concat.mutable_attr())["N"].set_i(count);

        tensorflow::NodeDef fused = first;
        fused.set_name(name + "/op");
        fused.clear_input();
        fused.add_input(concat.name());
        (*fused.mutable_attr())["_forward_input"].mutable_list()->add_i(0);

        tensorflow::NodeDef split;
        split.set_name(name + "/split");
        split.set_op("_ScopedAllocatorSplit");
        split.set_device(first.device());
        split.add_input(fused.name());
        (*split.mutable_attr())["T"].set_type(dtype);
        (*split.mutable_attr())["sa_name"].set_s(name);
        (*split.mutable_attr())["id"].set_i(id);
        (*split.mutable_attr())["N"].set_i(count);
        auto* ranks = (*split.mutable_attr())["ranks"].mutable_list();
        for (const int rank: m_scratch_ranks) {
            ranks->add_i(rank);
        }
        auto* dims = (*split.mutable_attr())["dims"].mutable_list();
        for (const auto dimension: m_scratch_dims) {
            dims->add_i(dimension);
        }

        for (int slot = 0; slot < count; ++slot) {
            const tensorflow::NodeDef& member =
                index.node(group.members[static_cast<std::size_t>(slot)]);
            m_scratch_removed.insert(member.name());
            m_scratch_redirects[member.name()] = split.name() + ":" + std::to_string(slot);
            m_scratch_redirects[member.name() + ":0"] = split.name() + ":" + std::to_string(slot);
        }
        m_scratch_new.push_back(std::move(allocator));
        m_scratch_new.push_back(std::move(concat));
        m_scratch_new.push_back(std::move(fused));
        m_scratch_new.push_back(std::move(split));
    }

    SyclPassContext& m_context;
    SyclTensorBytes m_bytes;
    int m_next_id{1};
    std::vector<Group> m_scratch_groups;
    std::vector<std::int64_t> m_scratch_dims;
    std::vector<int> m_scratch_ranks;
    std::set<std::string> m_scratch_removed;
    std::map<std::string, std::string> m_scratch_redirects;
    std::vector<tensorflow::NodeDef> m_scratch_new;
};

} // namespace aten_xpu
