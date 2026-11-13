module;

#include "include/c/intern/duration.h"
#include "include/c/intern/time_point.h"

export module aten_xpu_intern:time_point;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :duration;

export namespace aten_xpu {

// A time point stored as its duration since the epoch.
class TimePoint : public ice::builder::TimePoint
{
public:
    explicit TimePoint(const ::TF_DurationOps* duration_ops) noexcept :
        ice::builder::TimePoint{duration_ops}
    {
    }

    ~TimePoint() override = default;
    TimePoint(const TimePoint&) = delete;
    TimePoint& operator=(const TimePoint&) = delete;
    TimePoint(TimePoint&&) = delete;
    TimePoint& operator=(TimePoint&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void set(std::int64_t ticks, std::int64_t num, std::int64_t den) noexcept
    {
        m_ticks = ticks;
        m_num = num;
        m_den = den;
    }

    // Fills `out_duration` only when it is a Duration; any other duration is left untouched.
    void get_duration_since_epoch(const ice::sonic::Duration& out_duration) noexcept override
    {
        if (out_duration.get_handle()->plugin_data == nullptr) {
            return;
        }

        auto* target = dynamic_cast<Duration*>(
            &ice::builder::Duration::from_handle(out_duration.get_handle())
        );
        if (target != nullptr) {
            target->set(m_ticks, m_num, m_den);
        }
    }

private:
    std::int64_t m_ticks = 0;
    std::int64_t m_num = 1;
    std::int64_t m_den = 1;
};

} // namespace aten_xpu
