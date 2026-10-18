// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/grappler.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/grappler/grappler.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_grappler_builder:grappler;

import std;
import cc_ice_extern_grappler_sonic;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Grappler
{
public:
    explicit Grappler(
        const ::TFGrapplerDeviceGraphOps* GrapplerDeviceGraph_ops,
        const ::TF_DeviceOps* Device_ops,
        const ::TF_ExecutorOps* Executor_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_GrapplerDeviceGraph_ops = GrapplerDeviceGraph_ops;
        m_Device_ops = Device_ops;
        m_Executor_ops = Executor_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    Grappler(const Grappler&) = delete;
    Grappler& operator=(const Grappler&) = delete;

    static Grappler& from_handle(void* ctx) noexcept
    {
        return *static_cast<Grappler*>(ctx);
    }

    template<typename HandleT>
    static Grappler& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Grappler*>(handle->plugin_data);
    }

    virtual ~Grappler() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void create_device_graph_internal(
        const ice::sonic::Executor& executor,
        const ice::sonic::Device& device,
        const ice::sonic::GrapplerDeviceGraph& out_graph,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    destroy_device_graph_internal(const ice::sonic::GrapplerDeviceGraph& graph) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Grappler*)) noexcept
    {
        m_vtable = ::TF_GrapplerOps{
            .struct_size = TF_OFFSET_OF_END(::TF_GrapplerOps, destroy_device_graph_internal),

            .create = create,
            .destroy =
                [](::TF_Grappler* handle) noexcept
            {
                auto& self = Grappler::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TF_Grappler* grappler, ::TF_String* out_name) noexcept
            {
                auto& self = Grappler::from_handle(grappler);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .create_device_graph_internal =
                [](::TF_Grappler* grappler,
                   ::TF_Executor* executor,
                   ::TF_Device* device,
                   ::TFGrapplerDeviceGraph* out_graph,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Grappler::from_handle(grappler);
                self.create_device_graph_internal(
                    self.wrap(std::type_identity<ice::sonic::Executor>{}, executor),
                    self.wrap(std::type_identity<ice::sonic::Device>{}, device),
                    self.wrap(std::type_identity<ice::sonic::GrapplerDeviceGraph>{}, out_graph),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_device_graph_internal =
                [](::TF_Grappler* grappler, ::TFGrapplerDeviceGraph* graph) noexcept
            {
                auto& self = Grappler::from_handle(grappler);
                self.destroy_device_graph_internal(
                    self.wrap(std::type_identity<ice::sonic::GrapplerDeviceGraph>{}, graph)
                );
            },

        };
    }

    ice::sonic::GrapplerDeviceGraph wrap(
        std::type_identity<ice::sonic::GrapplerDeviceGraph>,
        const ::TFGrapplerDeviceGraph* handle
    ) const noexcept
    {
        return ice::sonic::GrapplerDeviceGraph{m_GrapplerDeviceGraph_ops, handle};
    }

    ice::sonic::Device
    wrap(std::type_identity<ice::sonic::Device>, const ::TF_Device* handle) const noexcept
    {
        return ice::sonic::Device{m_Device_ops, handle};
    }

    ice::sonic::Executor
    wrap(std::type_identity<ice::sonic::Executor>, const ::TF_Executor* handle) const noexcept
    {
        return ice::sonic::Executor{m_Executor_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TF_GrapplerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Grappler& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_GrapplerOps*>(&m_vtable));
    }

private:
    ::TF_GrapplerOps m_vtable;
    ::TF_Grappler m_handle;

    const ::TFGrapplerDeviceGraphOps* m_GrapplerDeviceGraph_ops{nullptr};

    const ::TF_DeviceOps* m_Device_ops{nullptr};

    const ::TF_ExecutorOps* m_Executor_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
