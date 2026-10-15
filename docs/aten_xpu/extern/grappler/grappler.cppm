module;

#include "include/c/extern/grappler/grappler.h"

export module aten_xpu_extern_grappler:grappler;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_grappler_builder;
import aten_xpu_intern;
import aten_xpu_extern_stream_executor;
import :device_graph;

export namespace aten_xpu {

class SyclGrappler : public ice::builder::Grappler
{
public:
    explicit SyclGrappler(const SyclOpsTable& ops) noexcept :
        ice::builder::Grappler{
            ops.getDeviceGraphOps(),
            ops.getDeviceOps(),
            ops.getExecutorOps(),
            ops.getStatusOps(),
            ops.getStringOps()
        },
        m_status{ops}
    {
    }

    ~SyclGrappler() override = default;
    SyclGrappler(const SyclGrappler&) = delete;
    SyclGrappler& operator=(const SyclGrappler&) = delete;
    SyclGrappler(SyclGrappler&&) = delete;
    SyclGrappler& operator=(SyclGrappler&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        m_status.copy_into(out_name, "xpu_grappler");
    }

    void create_device_graph_internal(
        const ice::sonic::Executor& executor,
        const ice::sonic::Device& device,
        const ice::sonic::GrapplerDeviceGraph& out_graph,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);
        auto& sycl_device = static_cast<SyclDevice&>(SyclDevice::from_handle(device.get_handle()));
        auto& sycl_executor =
            static_cast<SyclExecutor&>(SyclExecutor::from_handle(executor.get_handle()));
        static_cast<SyclDeviceGraph&>(SyclDeviceGraph::from_handle(out_graph.get_handle()))
            .bind(
                sycl_executor.getDefaultGeneratorState(sycl_device.getDeviceIndex()),
                sycl_device.getNativeDevice(),
                sycl_device.getDeviceIndex()
            );
    }

    void destroy_device_graph_internal(
        const ice::sonic::GrapplerDeviceGraph& graph
    ) noexcept override
    {
        auto& device_graph =
            static_cast<SyclDeviceGraph&>(SyclDeviceGraph::from_handle(graph.get_handle()));
        ice::sonic::Status ignored{m_status.getOps().getStatusOps()};
        device_graph.reset(ignored);
    }

private:
    SyclStatus m_status;
};

} // namespace aten_xpu
