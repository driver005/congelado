// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/event.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/event.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_stream_executor_builder:event;

import std;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Event
{
public:
    explicit Event(const ::TF_EventOps* Event_ops, const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Event_ops = Event_ops;
        m_Status_ops = Status_ops;
    }

    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    static Event& from_handle(void* ctx) noexcept
    {
        return *static_cast<Event*>(ctx);
    }

    template<typename HandleT>
    static Event& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Event*>(handle->plugin_data);
    }

    virtual ~Event() = default;
    virtual void destroy() noexcept = 0;
    virtual void elapsed_time(
        const ice::sonic::Event& end,
        float& out_milliseconds,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void export_ipc(
        ice::sonic::IpcEventHandle* out_handle,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_native_handle(void** out_handle) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Event*)) noexcept
    {
        m_vtable = ::TF_EventOps{
            .struct_size = TF_OFFSET_OF_END(::TF_EventOps, get_native_handle),

            .create = create,
            .destroy =
                [](::TF_Event* handle) noexcept
            {
                auto& self = Event::from_handle(handle);
                self.destroy();
            },
            .elapsed_time =
                [](::TF_Event* start,
                   ::TF_Event* end,
                   float* out_milliseconds,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Event::from_handle(start);
                self.elapsed_time(
                    self.wrap(std::type_identity<ice::sonic::Event>{}, end),
                    *out_milliseconds,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .export_ipc =
                [](::TF_Event* event,
                   ::TF_IpcEventHandle* out_handle,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = Event::from_handle(event);
                std::optional<ice::sonic::IpcEventHandle> wrapped_out_handle;
                if (out_handle != nullptr) {
                    wrapped_out_handle.emplace(*out_handle);
                }
                self.export_ipc(
                    wrapped_out_handle ? &*wrapped_out_handle : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (out_handle != nullptr) {
                    *out_handle = *wrapped_out_handle->get_handle();
                }
            },
            .get_native_handle =
                [](::TF_Event* event, void** out_handle) noexcept
            {
                auto& self = Event::from_handle(event);
                self.get_native_handle(out_handle);
            },

        };
    }

    ice::sonic::Event
    wrap(std::type_identity<ice::sonic::Event>, const ::TF_Event* handle) const noexcept
    {
        return ice::sonic::Event{m_Event_ops, handle};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
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
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_EventOps*>(&m_vtable));
    }

private:
    ::TF_EventOps m_vtable;
    ::TF_Event m_handle;

    const ::TF_EventOps* m_Event_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
