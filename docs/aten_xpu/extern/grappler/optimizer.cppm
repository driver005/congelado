module;

#include "cc/proto/graph.pb.h"
#include "include/c/extern/grappler/optimizer.h"

export module aten_xpu_extern_grappler:optimizer;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_grappler_builder;
import aten_xpu_intern;
import :properties;
import :op_lookup;
import :pass_context;
import :pipeline;

export namespace aten_xpu {

class SyclGrapplerOptimizer : public ice::builder::GrapplerOptimizer
{
public:
    explicit SyclGrapplerOptimizer(const SyclOpsTable& ops) noexcept :
        ice::builder::GrapplerOptimizer{
            ops.getGrapplerItemOps(),
            ops.getBufferOps(),
            ops.getStatusOps()
        },
        m_ops{ops},
        m_status{ops}
    {
    }

    ~SyclGrapplerOptimizer() override = default;
    SyclGrapplerOptimizer(const SyclGrapplerOptimizer&) = delete;
    SyclGrapplerOptimizer& operator=(const SyclGrapplerOptimizer&) = delete;
    SyclGrapplerOptimizer(SyclGrapplerOptimizer&&) = delete;
    SyclGrapplerOptimizer& operator=(SyclGrapplerOptimizer&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void optimize(
        const ice::sonic::Buffer& graph_buf,
        const ice::sonic::GrapplerItem& item,
        const ice::sonic::Buffer& out_optimized_graph_buf,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        TFBufferData input{};
        graph_buf.get_buffer(&input);

        tensorflow::GraphDef graph;
        if (!graph.ParseFromArray(input.data, static_cast<int>(input.length))) {
            m_status.fail(out_status, ice::sonic::Code::k_invalid_argument, "malformed GraphDef");
            return;
        }

        m_preserved.clear();
        m_fetch.clear();
        collect_names(item, out_status, false, m_preserved);
        collect_names(item, out_status, true, m_fetch);

        // Shape inference runs the shape function of every op, so the passes see the shapes the
        // host's registry defines; the op lookup shows which attrs and flags those ops declare.
        tensorflow::GraphDef fused = graph;
        try {
            SyclGraphProperties properties{m_ops, item};
            properties.infer();
            SyclOpLookup lookup{m_ops, graph_buf};
            SyclPassContext context{properties, lookup};
            context.setPreserved(m_preserved);
            context.setFetch(m_fetch);
            context.setConfigs(m_ops.getGrapplerOptimizerConfigs());
            context.setDeviceInfo(
                m_ops.getDeviceMemoryBytes(),
                m_ops.getDeviceBandwidth(),
                m_ops.getDeviceCount()
            );
            SyclGraphPipeline pipeline{context};
            pipeline.run(fused);
        } catch (const std::exception& error) {
            m_status.fail(out_status, ice::sonic::Code::k_internal, error.what());
            return;
        }
        if (!fused.SerializeToString(&m_scratch_bytes)) {
            m_status.fail(out_status, ice::sonic::Code::k_internal, "failed to serialize GraphDef");
            return;
        }
        out_optimized_graph_buf.assign_from_string(m_scratch_bytes.data(), m_scratch_bytes.size());
    }

private:
    void collect_names(
        const ice::sonic::GrapplerItem& item,
        const ice::sonic::Status& status,
        bool fetch_nodes,
        std::set<std::string>& names
    )
    {
        int count = 0;
        std::size_t storage_size = 0;
        if (fetch_nodes) {
            item.get_fetch_nodes_size(count, storage_size, status);
        } else {
            item.get_nodes_to_preserve_size(count, storage_size, status);
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
            names.emplace(m_scratch_values[index], m_scratch_lengths[index]);
        }
    }

    const SyclOpsTable& m_ops;
    SyclStatus m_status;
    std::set<std::string> m_preserved;
    std::set<std::string> m_fetch;
    std::vector<char*> m_scratch_values;
    std::vector<std::size_t> m_scratch_lengths;
    std::vector<std::byte> m_scratch_storage;
    std::string m_scratch_bytes;
};

} // namespace aten_xpu
