// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/grappler.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/grappler/grappler.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_grappler_builder:grappler;

import std;
import cc_ice_extern_grappler_sonic;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_GrapplerOps
{
public:
    explicit TF_GrapplerOps(
        const ::TFGrapplerDeviceGraphOps* TFGrapplerDeviceGraphOps_ops,
        const ::TF_DeviceOps* TF_DeviceOps_ops,
        const ::TF_ExecutorOps* TF_ExecutorOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TFGrapplerDeviceGraphOps_ops = TFGrapplerDeviceGraphOps_ops;
        m_TF_DeviceOps_ops = TF_DeviceOps_ops;
        m_TF_ExecutorOps_ops = TF_ExecutorOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void create_device_graph_internal(
        const ice::sonic::TF_ExecutorOps& executor,
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::TFGrapplerDeviceGraphOps& out_graph,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    destroy_device_graph_internal(const ice::sonic::TFGrapplerDeviceGraphOps& graph) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Grappler*)) noexcept
    {
        m_vtable = ::TF_GrapplerOps{
            .struct_size = TF_OFFSET_OF_END(::TF_GrapplerOps, destroy_device_graph_internal),

            .create = create,
            .destroy =
                [](TF_Grappler* handle) noexcept
            {
                auto& self = TF_GrapplerOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Grappler* grappler, TF_String* out_name) noexcept
            {
                auto& self = TF_GrapplerOps::from_handle(grappler);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .create_device_graph_internal =
                [](TF_Grappler* grappler,
                   TF_Executor* executor,
                   TF_Device* device,
                   TFGrapplerDeviceGraph* out_graph,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_GrapplerOps::from_handle(grappler);
                self.create_device_graph_internal(
                    self.wrap(std::type_identity<ice::sonic::TF_ExecutorOps>{}, executor),
                    self.wrap(std::type_identity<ice::sonic::TF_DeviceOps>{}, device),
                    self.wrap(
                        std::type_identity<ice::sonic::TFGrapplerDeviceGraphOps>{},
                        out_graph
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_device_graph_internal =
                [](TF_Grappler* grappler, TFGrapplerDeviceGraph* graph) noexcept
            {
                auto& self = TF_GrapplerOps::from_handle(grappler);
                self.destroy_device_graph_internal(
                    self.wrap(std::type_identity<ice::sonic::TFGrapplerDeviceGraphOps>{}, graph)
                );
            },

        };
    }

    ice::sonic::TFGrapplerDeviceGraphOps wrap(
        std::type_identity<ice::sonic::TFGrapplerDeviceGraphOps>,
        const ::TFGrapplerDeviceGraph* handle
    ) const noexcept
    {
        return ice::sonic::TFGrapplerDeviceGraphOps{
            m_TFGrapplerDeviceGraphOps_ops,
            const_cast<::TFGrapplerDeviceGraph*>(handle)
        };
    }

    ice::sonic::TF_DeviceOps
    wrap(std::type_identity<ice::sonic::TF_DeviceOps>, const ::TF_Device* handle) const noexcept
    {
        return ice::sonic::TF_DeviceOps{m_TF_DeviceOps_ops, const_cast<::TF_Device*>(handle)};
    }

    ice::sonic::TF_ExecutorOps
    wrap(std::type_identity<ice::sonic::TF_ExecutorOps>, const ::TF_Executor* handle) const noexcept
    {
        return ice::sonic::TF_ExecutorOps{m_TF_ExecutorOps_ops, const_cast<::TF_Executor*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
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
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_GrapplerOps*>(&m_vtable)
        );
    }

private:
    ::TF_GrapplerOps m_vtable;
    ::TF_Grappler m_handle;

    const ::TFGrapplerDeviceGraphOps* m_TFGrapplerDeviceGraphOps_ops{nullptr};

    const ::TF_DeviceOps* m_TF_DeviceOps_ops{nullptr};

    const ::TF_ExecutorOps* m_TF_ExecutorOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
