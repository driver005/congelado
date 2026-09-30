// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/event.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/event.h"

export module cc_ice_extern_stream_executor_builder:event;

import std;

export namespace ice::builder {

class TF_EventOps
{
public:
    TF_EventOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::Status>
    elapsed_time(const ice::sonic::TF_EventOps& end, float* out_milliseconds) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    export_ipc(TF_IpcEventHandle* out_handle) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_native_handle(void** out_handle) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_EventOps{
            .struct_size = TF_EVENT_STRUCT_SIZE,
            .elapsed_time =
                [](TF_Event* start,
                   TF_Event* end,
                   float* out_milliseconds,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_EventOps::from_handle(start).elapsed_time(
                    ice::sonic::TF_EventOps::wrap(end),
                    out_milliseconds
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .export_ipc =
                [](TF_Event* event, TF_IpcEventHandle* out_handle, TF_Status* out_status) noexcept
            {
                auto res = TF_EventOps::from_handle(event).export_ipc(out_handle);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_native_handle =
                [](TF_Event* event, void** out_handle) noexcept
            {
                auto res = TF_EventOps::from_handle(event).get_native_handle(out_handle);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_EventOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Event& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_EventOps m_vtable;
    TF_Event m_handle;
};

} // namespace ice::builder
