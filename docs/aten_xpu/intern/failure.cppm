module;

#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:failure;

import std;
import cc_ice_intern_sonic;

export namespace aten_xpu {

// Fills a caller supplied status with a code and message built from the string ops.
class Failure
{
public:
    explicit Failure(const ::TF_StringOps* string_ops) noexcept :
        m_string_ops{string_ops}
    {
    }

    void fail(
        const ice::sonic::Status& status,
        ice::sonic::Code code,
        std::string_view message
    ) const noexcept
    {
        ice::sonic::String text{m_string_ops};
        text.create();
        text.copy(message);
        status.set_status(code, text);
        text.destroy();
    }

    // Maps an errno value to a status code; the message is "<context>: <errno text>".
    void fail_errno(
        const ice::sonic::Status& status,
        int error_code,
        std::string_view context
    ) const noexcept
    {
        ice::sonic::String text{m_string_ops};
        text.create();
        text.copy(context);
        status.set_status_from_io_error(error_code, text);
        text.destroy();
    }

private:
    const ::TF_StringOps* m_string_ops;
};

} // namespace aten_xpu
