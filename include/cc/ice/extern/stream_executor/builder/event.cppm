// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/event.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/extern/stream_executor/event.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_stream_executor_builder:event;

import std;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_EventOps
{
public:
    explicit TF_EventOps(
        const ::TF_EventOps* TF_EventOps_ops,
        const ::TF_StatusOps* Status_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_EventOps_ops = TF_EventOps_ops;
        m_Status_ops = Status_ops;
    }

    TF_EventOps(const TF_EventOps&) = delete;
    TF_EventOps& operator=(const TF_EventOps&) = delete;

    static TF_EventOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_EventOps*>(ctx);
    }

    template<typename HandleT>
    static TF_EventOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_EventOps*>(handle->plugin_data);
    }

    virtual ~TF_EventOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void elapsed_time(
        const ice::sonic::TF_EventOps& end,
        float* out_milliseconds,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    export_ipc(TF_IpcEventHandle* out_handle, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_native_handle(void** out_handle) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Event*)) noexcept
    {
        m_vtable = ::TF_EventOps{
            .struct_size = TF_OFFSET_OF_END(::TF_EventOps, get_native_handle),

            .create = create,
            .destroy =
                [](TF_Event* handle) noexcept
            {
                auto& self = TF_EventOps::from_handle(handle);
                self.destroy();
            },
            .elapsed_time =
                [](TF_Event* start,
                   TF_Event* end,
                   float* out_milliseconds,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_EventOps::from_handle(start);
                self.elapsed_time(
                    self.wrap(std::type_identity<ice::sonic::TF_EventOps>{}, end),
                    out_milliseconds,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .export_ipc =
                [](TF_Event* event, TF_IpcEventHandle* out_handle, TF_Status* out_status) noexcept
            {
                auto& self = TF_EventOps::from_handle(event);
                self.export_ipc(
                    out_handle,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_native_handle =
                [](TF_Event* event, void** out_handle) noexcept
            {
                auto& self = TF_EventOps::from_handle(event);
                self.get_native_handle(out_handle);
            },

        };
    }

    ice::sonic::TF_EventOps
    wrap(std::type_identity<ice::sonic::TF_EventOps>, const ::TF_Event* handle) const noexcept
    {
        return ice::sonic::TF_EventOps{m_TF_EventOps_ops, const_cast<::TF_Event*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_EventOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Event& get_handle() const noexcept
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
            const_cast<::TF_EventOps*>(&m_vtable)
        );
    }

private:
    ::TF_EventOps m_vtable;
    ::TF_Event m_handle;

    const ::TF_EventOps* m_TF_EventOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
