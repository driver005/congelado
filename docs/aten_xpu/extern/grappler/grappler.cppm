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

class SyclGrappler : public ice::builder::TF_GrapplerOps
{
public:
    explicit SyclGrappler(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_GrapplerOps{
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

    static void create(::TF_Grappler* handle)
    {
        auto* grappler = new SyclGrappler{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *grappler);
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        m_status.copy_into(out_name, "xpu_grappler");
    }

    void create_device_graph_internal(
        const ice::sonic::TF_ExecutorOps& executor,
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TFGrapplerDeviceGraphOps& out_graph,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);
        auto& sycl_device = SyclHandle::resolve<SyclDevice>(device);
        auto& sycl_executor = SyclHandle::resolve<SyclExecutor>(executor);
        SyclHandle::resolve<SyclDeviceGraph>(out_graph).bind(
            sycl_executor.getDefaultGeneratorState(sycl_device.getDeviceIndex()),
            sycl_device.getNativeDevice(),
            sycl_device.getDeviceIndex()
        );
    }

    void destroy_device_graph_internal(
        const ice::sonic::TFGrapplerDeviceGraphOps& graph
    ) noexcept override
    {
        auto& device_graph = SyclHandle::resolve<SyclDeviceGraph>(graph);
        ice::sonic::Status ignored{m_status.getOps().getStatusOps()};
        device_graph.reset(ignored);
    }

private:
    SyclStatus m_status;
};

} // namespace aten_xpu
