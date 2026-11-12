module;

#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_profiler_common:status;

import std;
import cc_ice_intern_sonic;
import :host_ops;

export namespace aten_xpu {

class ProfilerStatus
{
public:
    explicit ProfilerStatus(const ProfilerHostOps& ops) noexcept :
        m_ops{ops}
    {
    }

    void fail(
        const ice::sonic::Status& status,
        ice::sonic::Code code,
        std::string_view message
    ) const noexcept
    {
        ice::sonic::String text{m_ops.getStringOps()};
        text.create();
        text.copy(message);
        status.set_status(code, text);
        text.destroy();
    }

private:
    const ProfilerHostOps& m_ops;
};

} // namespace aten_xpu
