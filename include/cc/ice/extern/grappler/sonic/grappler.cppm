// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/grappler.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/grappler.h"

export module cc_ice_extern_grappler_sonic:grappler;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_GrapplerOps : public ice::sonic::Runtime<TF_GrapplerOps, TF_GrapplerOps>
{
public:
    explicit TF_GrapplerOps(TF_GrapplerOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "grappler";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> create_device_graph_internal(
        const ice::sonic::TF_ExecutorOps& executor,
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TFGrapplerDeviceGraphOps& out_graph
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->create_device_graph_internal(
            get_handle(),
            executor.get_handle(),
            device.get_handle(),
            out_graph.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void destroy_device_graph_internal(const ice::sonic::TFGrapplerDeviceGraphOps& graph) noexcept
    {
        m_ops->destroy_device_graph_internal(get_handle(), graph.get_handle());
    }
};

} // namespace ice::sonic
