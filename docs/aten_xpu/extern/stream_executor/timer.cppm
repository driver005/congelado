module;

#include "include/c/extern/stream_executor/timer.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:timer;

import std;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclTimer : public ice::builder::TF_TimerOps
{
public:
    SyclTimer() noexcept = default;

    ~SyclTimer() override = default;
    SyclTimer(const SyclTimer&) = delete;
    SyclTimer& operator=(const SyclTimer&) = delete;
    SyclTimer(SyclTimer&&) = delete;
    SyclTimer& operator=(SyclTimer&&) = delete;

    static void create(::TF_Timer* handle)
    {
        auto* timer = new SyclTimer{};
        SyclHandle::attach(handle, *timer);
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void nanoseconds(uint64_t* out_nanoseconds) noexcept override
    {
        if (!m_start || !m_stop) {
            *out_nanoseconds = 0;
            return;
        }

        m_stop->wait();
        const auto start_nanoseconds =
            m_start->get_profiling_info<sycl::info::event_profiling::command_end>();
        const auto stop_nanoseconds =
            m_stop->get_profiling_info<sycl::info::event_profiling::command_end>();
        *out_nanoseconds =
            stop_nanoseconds > start_nanoseconds ? stop_nanoseconds - start_nanoseconds : 0;
    }

    void setStartEvent(sycl::event event) noexcept
    {
        m_start = std::move(event);
    }

    void setStopEvent(sycl::event event) noexcept
    {
        m_stop = std::move(event);
    }

private:
    std::optional<sycl::event> m_start;
    std::optional<sycl::event> m_stop;
};

} // namespace aten_xpu
