module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/graph.pb.h"
#include "cc/proto/node_def.pb.h"
#include "include/c/extern/grappler/optimizer.h"

export module aten_xpu_extern_grappler:optimizer;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_grappler_builder;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclGrapplerOptimizer : public ice::builder::TFGrapplerOptimizerOps
{
public:
    explicit SyclGrapplerOptimizer(const SyclOpsTable& ops) noexcept :
        ice::builder::TFGrapplerOptimizerOps{
            ops.getGrapplerItemOps(),
            ops.getBufferOps(),
            ops.getStatusOps()
        },
        m_status{ops}
    {
    }

    ~SyclGrapplerOptimizer() override = default;
    SyclGrapplerOptimizer(const SyclGrapplerOptimizer&) = delete;
    SyclGrapplerOptimizer& operator=(const SyclGrapplerOptimizer&) = delete;
    SyclGrapplerOptimizer(SyclGrapplerOptimizer&&) = delete;
    SyclGrapplerOptimizer& operator=(SyclGrapplerOptimizer&&) = delete;

    static void create(::TFGrapplerOptimizer* handle)
    {
        auto* optimizer = new SyclGrapplerOptimizer{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *optimizer);
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void optimize(
        const ice::sonic::TF_BufferOps& graph_buf,
        const ice::sonic::TFGrapplerItemOps& item,
        const ice::sonic::TF_BufferOps& out_optimized_graph_buf,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        TFBufferData input{};
        graph_buf.get_buffer(&input);

        tensorflow::GraphDef graph;
        if (!graph.ParseFromArray(input.data, static_cast<int>(input.length))) {
            m_status.fail(out_status, TF_INVALID_ARGUMENT, "malformed GraphDef");
            return;
        }

        m_protected.clear();
        collect_protected(item, out_status, false);
        collect_protected(item, out_status, true);

        const auto fused = fuse_bias_and_activation(graph);
        if (!fused.SerializeToString(&m_scratch_bytes)) {
            m_status.fail(out_status, TF_INTERNAL, "failed to serialize GraphDef");
            return;
        }
        out_optimized_graph_buf.assign_from_string(m_scratch_bytes.data(), m_scratch_bytes.size());
    }

private:
    void collect_protected(
        const ice::sonic::TFGrapplerItemOps& item,
        const ice::sonic::Status& status,
        bool fetch_nodes
    )
    {
        int count = 0;
        std::size_t storage_size = 0;
        if (fetch_nodes) {
            item.get_fetch_nodes_size(&count, &storage_size, status);
        } else {
            item.get_nodes_to_preserve_size(&count, &storage_size, status);
        }
        if (count <= 0) {
            return;
        }

        m_scratch_values.assign(static_cast<std::size_t>(count), nullptr);
        m_scratch_lengths.assign(static_cast<std::size_t>(count), 0);
        m_scratch_storage.resize(storage_size);
        if (fetch_nodes) {
            item.get_fetch_nodes_list(
                m_scratch_values.data(),
                m_scratch_lengths.data(),
                count,
                m_scratch_storage.data(),
                storage_size,
                status
            );
        } else {
            item.get_nodes_to_preserve_list(
                m_scratch_values.data(),
                m_scratch_lengths.data(),
                count,
                m_scratch_storage.data(),
                storage_size,
                status
            );
        }

        for (std::size_t index = 0; index < m_scratch_values.size(); ++index) {
            m_protected.emplace(m_scratch_values[index], m_scratch_lengths[index]);
        }
    }

    static bool is_fusable_producer(const std::string& op) noexcept
    {
        return op == "Conv2D" || op == "Addmm" || op == "Bmm" || op == "MatMul";
    }

    static int count_consumers(const tensorflow::GraphDef& graph, const std::string& output)
    {
        int count = 0;
        for (const auto& node: graph.node()) {
            count += static_cast<int>(std::ranges::count(node.input(), output));
        }
        return count;
    }

    static const tensorflow::NodeDef*
    find_node(const tensorflow::GraphDef& graph, const std::string& name)
    {
        const auto found = std::ranges::find_if(
            graph.node(),
            [&name](const tensorflow::NodeDef& node)
            {
                return node.name() == name;
            }
        );
        return found == graph.node().end() ? nullptr : &*found;
    }

    static const tensorflow::NodeDef*
    find_relu_consumer(const tensorflow::GraphDef& graph, const std::string& output)
    {
        const auto found = std::ranges::find_if(
            graph.node(),
            [&output](const tensorflow::NodeDef& node)
            {
                return node.op() == "Relu" && node.input_size() == 1 && node.input(0) == output;
            }
        );
        return found == graph.node().end() ? nullptr : &*found;
    }

    tensorflow::GraphDef fuse_bias_and_activation(const tensorflow::GraphDef& input)
    {
        m_scratch_absorbed.clear();
        m_scratch_replacements.clear();

        for (const auto& node: input.node()) {
            if (node.op() != "BiasAdd" || node.input_size() == 0 ||
                m_protected.contains(node.name())) {
                continue;
            }

            const auto* producer = find_node(input, node.input(0));
            if (producer == nullptr || !is_fusable_producer(producer->op()) ||
                m_protected.contains(producer->name()) ||
                m_scratch_absorbed.contains(producer->name()) ||
                count_consumers(input, producer->name()) != 1) {
                continue;
            }

            tensorflow::NodeDef fused = *producer;
            if (node.input_size() > 1) {
                fused.add_input(node.input(1));
            }
            fused.set_name(node.name());
            m_scratch_absorbed.insert(producer->name());

            const auto* relu = count_consumers(input, node.name()) == 1
                                   ? find_relu_consumer(input, node.name())
                                   : nullptr;
            if (relu != nullptr && !m_protected.contains(relu->name())) {
                (*fused.mutable_attr())["activation"].set_i(1);
                fused.set_name(relu->name());
                m_scratch_absorbed.insert(node.name());
                m_scratch_replacements.emplace(relu->name(), std::move(fused));
                continue;
            }
            m_scratch_replacements.emplace(node.name(), std::move(fused));
        }

        tensorflow::GraphDef output;
        *output.mutable_versions() = input.versions();
        *output.mutable_library() = input.library();
        for (const auto& node: input.node()) {
            if (m_scratch_absorbed.contains(node.name())) {
                continue;
            }
            const auto replacement = m_scratch_replacements.find(node.name());
            *output.add_node() =
                replacement == m_scratch_replacements.end() ? node : replacement->second;
        }
        return output;
    }

    SyclStatus m_status;
    std::set<std::string> m_protected;
    std::set<std::string> m_scratch_absorbed;
    std::map<std::string, tensorflow::NodeDef> m_scratch_replacements;
    std::vector<char*> m_scratch_values;
    std::vector<std::size_t> m_scratch_lengths;
    std::vector<std::byte> m_scratch_storage;
    std::string m_scratch_bytes;
};

} // namespace aten_xpu
