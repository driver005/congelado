// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/grappler.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/grappler.h"

export module cc_ice_extern_grappler_builder:grappler;

import std;

export namespace ice::builder {

class TF_GrapplerOps
{
public:
    TF_GrapplerOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_GrapplerOps(const TF_GrapplerOps&) = delete;
    TF_GrapplerOps& operator=(const TF_GrapplerOps&) = delete;

    static TF_GrapplerOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_GrapplerOps*>(ctx);
    }

    template<typename HandleT>
    static TF_GrapplerOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_GrapplerOps*>(handle->plugin_data);
    }

    virtual ~TF_GrapplerOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> create_device_graph_internal(
        const ice::sonic::TF_ExecutorOps& executor,
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TFGrapplerDeviceGraphOps& out_graph
    ) noexcept = 0;
    virtual void
    destroy_device_graph_internal(const ice::sonic::TFGrapplerDeviceGraphOps& graph) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_GrapplerOps{
            .struct_size = TF_GRAPPLER_STRUCT_SIZE,
            .destroy =
                [](TF_Grappler* grappler) noexcept
            {
                TF_GrapplerOps::from_handle(grappler).destroy();
            },
            .get_name =
                [](TF_Grappler* grappler, TF_String* out_name) noexcept
            {
                TF_GrapplerOps::from_handle(grappler).get_name(ice::sonic::String::wrap(out_name));
            },
            .create_device_graph_internal =
                [](TF_Grappler* grappler,
                   TF_Executor* executor,
                   TF_Device* device,
                   TFGrapplerDeviceGraph* out_graph,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_GrapplerOps::from_handle(grappler).create_device_graph_internal(
                    ice::sonic::TF_ExecutorOps::wrap(executor),
                    ice::sonic::TF_DeviceOps::wrap(device),
                    ice::sonic::TFGrapplerDeviceGraphOps::wrap(out_graph)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_device_graph_internal =
                [](TF_Grappler* grappler, TFGrapplerDeviceGraph* graph) noexcept
            {
                TF_GrapplerOps::from_handle(grappler).destroy_device_graph_internal(
                    ice::sonic::TFGrapplerDeviceGraphOps::wrap(graph)
                );
            },

        };
    }

    const ::TF_GrapplerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Grappler& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_GrapplerOps m_vtable;
    TF_Grappler m_handle;
};

} // namespace ice::builder
