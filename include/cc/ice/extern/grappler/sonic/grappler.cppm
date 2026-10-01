// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/grappler.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/grappler.h"

export module cc_ice_extern_grappler_sonic:grappler;

import std;
import :device_graph;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_GrapplerOps : public ice::sonic::Runtime<::TF_GrapplerOps, ::TF_Grappler>
{
public:
    template<typename Registry>
    TF_GrapplerOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_GrapplerOps(
        Registry& registry,
        ::TF_Grappler* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_GrapplerOps(const ::TF_GrapplerOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_GrapplerOps(const ::TF_GrapplerOps* ops, ::TF_Grappler* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void create_device_graph_internal(
        const ice::sonic::TF_ExecutorOps& executor,
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TFGrapplerDeviceGraphOps& out_graph,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_device_graph_internal(
            get_handle(),
            executor.get_handle(),
            device.get_handle(),
            out_graph.get_handle(),
            out_status.get_handle()
        );
    }

    void destroy_device_graph_internal(
        const ice::sonic::TFGrapplerDeviceGraphOps& graph
    ) const noexcept
    {
        m_ops->destroy_device_graph_internal(get_handle(), graph.get_handle());
    }
};

} // namespace ice::sonic
