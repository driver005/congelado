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
    static TF_EventOps* create(void* ctx) noexcept
    {
        return static_cast<TF_EventOps*>(ctx);
    }

    template<typename HandleT>
    static TF_EventOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_EventOps*>(handle->plugin_data);
    }

    virtual ~TF_EventOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    elapsed_time(const ice::sonic::TF_EventOps& end, float* out_milliseconds) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    export_ipc(TF_IpcEventHandle* out_handle) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_native_handle(void** out_handle) noexcept = 0;

    static TF_EventOps* get_generic_vtable()
    {
        static TF_EventOps vtable = {
            .struct_size = TF_EVENT_STRUCT_SIZE,
            .elapsed_time =
                [](TF_Event* start,
                   TF_Event* end,
                   float* out_milliseconds,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_EventOps::create(start);
                auto res = self->elapsed_time(ice::sonic::TF_EventOps::wrap(end), out_milliseconds);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .export_ipc =
                [](TF_Event* event, TF_IpcEventHandle* out_handle, TF_Status* out_status) noexcept
            {
                auto* self = TF_EventOps::create(event);
                auto res = self->export_ipc(out_handle);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_native_handle =
                [](TF_Event* event, void** out_handle) noexcept
            {
                auto* self = TF_EventOps::create(event);
                auto res = self->get_native_handle(out_handle);
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
