// SYCL reference plugin — a start/stop sycl::event pair, timed by profiling info.
//
// Not built by Bazel (docs/ only). TF_TimerOps has one slot (nanoseconds); the start/stop
// lifecycle lives on TF_ExecutorOps::start_timer/stop_timer, which record the two events this
// class holds.

module;

#include "include/c/extern/stream_executor/timer.h"

export module sycl_backend:timer;

import std;
import cc_ice_extern_stream_executor_builder;

export namespace sycl_backend {

class SyclTimer : public ice::builder::Timer
{
public:
    SyclTimer() = default;

    ~SyclTimer() override = default;
    SyclTimer(const SyclTimer&) = delete;
    SyclTimer& operator=(const SyclTimer&) = delete;
    SyclTimer(SyclTimer&&) = delete;
    SyclTimer& operator=(SyclTimer&&) = delete;

    void set_start_event(sycl::event&& event) noexcept
    {
        m_start = std::move(event);
    }

    void set_stop_event(sycl::event&& event) noexcept
    {
        m_stop = std::move(event);
    }

    void nanoseconds(uint64_t* out_nanoseconds) noexcept override
    {
        const auto start_ns =
            m_start.get_profiling_info<sycl::info::event_profiling::command_end>();
        const auto stop_ns = m_stop.get_profiling_info<sycl::info::event_profiling::command_end>();
        *out_nanoseconds = stop_ns > start_ns ? stop_ns - start_ns : 0;
    }

private:
    sycl::event m_start;
    sycl::event m_stop;
};

} // namespace sycl_backend
