module;

#include "include/c/intern/duration.h"

export module aten_xpu_intern:duration;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;

export namespace aten_xpu {

// A duration of `ticks` periods of num/den seconds; the ABI only reads it, so values come from
// here.
class Duration : public ice::builder::Duration
{
public:
    Duration() noexcept = default;

    Duration(std::int64_t ticks, std::int64_t num, std::int64_t den) noexcept :
        m_ticks{ticks},
        m_num{num},
        m_den{den}
    {
    }

    ~Duration() override = default;
    Duration(const Duration&) = delete;
    Duration& operator=(const Duration&) = delete;
    Duration(Duration&&) = delete;
    Duration& operator=(Duration&&) = delete;

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

    void get_ticks(std::int64_t& out_ticks) noexcept override
    {
        out_ticks = m_ticks;
    }

    void get_ratio_num(std::int64_t& out_num) noexcept override
    {
        out_num = m_num;
    }

    void get_ratio_den(std::int64_t& out_den) noexcept override
    {
        out_den = m_den;
    }

private:
    std::int64_t m_ticks = 0;
    std::int64_t m_num = 1;
    std::int64_t m_den = 1;
};

} // namespace aten_xpu
