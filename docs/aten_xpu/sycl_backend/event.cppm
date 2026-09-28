// SYCL reference plugin — one sycl::event per TF_Event.
//
// Not built by Bazel (docs/ only). Replaces the lazily-created XPUEvent in c10/XPUEvent.h; this
// event is created eagerly by SyclExecutor::create_event_with_options_internal, since the C ABI
// has no "not yet recorded" state to lazily populate later — record_event (an executor slot)
// just re-submits the profiling barrier on an already-live sycl::event.

module;

#include "include/c/extern/stream_executor/event.h"

export module sycl_backend:event;

import std;
import cc_ice_extern_stream_executor_builder;

export namespace sycl_backend {

class SyclEvent : public ice::builder::Event
{
public:
    explicit SyclEvent(bool enable_timing) noexcept :
        m_enable_timing{enable_timing}
    {
    }

    ~SyclEvent() override = default;
    SyclEvent(const SyclEvent&) = delete;
    SyclEvent& operator=(const SyclEvent&) = delete;
    SyclEvent(SyclEvent&&) = delete;
    SyclEvent& operator=(SyclEvent&&) = delete;

    sycl::event& get_native_event() noexcept
    {
        return m_event;
    }

    void set_native_event(sycl::event&& event) noexcept
    {
        m_event = std::move(event);
    }

    bool timing_enabled() const noexcept
    {
        return m_enable_timing;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    elapsed_time(ice::builder::Event& end, float* out_milliseconds) noexcept override
    {
        // end was created by this same plugin, same as every Device/Stream/Event handed to a
        // SyclExecutor slot — see executor.cppm's as_event() note.
        auto& native_end = static_cast<SyclEvent&>(end);

        if (!m_enable_timing || !native_end.timing_enabled()) {
            return std::unexpected{
                ice::sonic::Status::from_message("SyclEvent: elapsed_time needs enable_timing on both events")
            };
        }

        try {
            const auto start_ns =
                m_event.get_profiling_info<sycl::info::event_profiling::command_end>();
            const auto end_ns = native_end.get_native_event()
                                     .get_profiling_info<sycl::info::event_profiling::command_end>();
            *out_milliseconds = static_cast<float>(end_ns - start_ns) / 1'000'000.0F;
        } catch (const sycl::exception& error) {
            return std::unexpected{ice::sonic::Status::from_message(error.what())};
        }

        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    export_ipc(TF_IpcEventHandle* out_handle) noexcept override
    {
        // IPC events need SYCL_COMPILER_VERSION >= 20260200's syclex::ipc::event API; out of
        // scope for this reference backend.
        (void)out_handle;
        return std::unexpected{ice::sonic::Status::from_message("SyclEvent: IPC export not implemented")};
    }

    void get_native_handle(void** out_handle) noexcept override
    {
        *out_handle = &m_event;
    }

private:
    sycl::event m_event;
    bool m_enable_timing;
};

} // namespace sycl_backend
