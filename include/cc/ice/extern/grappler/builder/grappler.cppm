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
    static TF_GrapplerOps* create(void* ctx) noexcept
    {
        return static_cast<TF_GrapplerOps*>(ctx);
    }

    template<typename HandleT>
    static TF_GrapplerOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_GrapplerOps*>(handle->plugin_data);
    }

    virtual ~TF_GrapplerOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> create_device_graph_internal(
        const ice::sonic::TF_ExecutorOps& executor,
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TFGrapplerDeviceGraphOps& out_graph
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    destroy_device_graph_internal(const ice::sonic::TFGrapplerDeviceGraphOps& graph) noexcept = 0;

    static TF_GrapplerOps* get_generic_vtable()
    {
        static TF_GrapplerOps vtable = {
            .struct_size = TF_GRAPPLER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_GrapplerOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_GrapplerOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .create_device_graph_internal =
                [](TF_Grappler* grappler,
                   TF_Executor* executor,
                   TF_Device* device,
                   TFGrapplerDeviceGraph* out_graph,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_GrapplerOps::create(grappler);
                auto res = self->create_device_graph_internal(
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
                auto* self = TF_GrapplerOps::create(grappler);
                auto res = self->destroy_device_graph_internal(
                    ice::sonic::TFGrapplerDeviceGraphOps::wrap(graph)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
