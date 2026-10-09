module;

#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <sycl/sycl.hpp>

export module aten_xpu_intern:status;

import std;
import cc_ice_intern_sonic;
import :ops_table;

export namespace aten_xpu {

class SyclStatus
{
public:
    explicit SyclStatus(const SyclOpsTable& ops) noexcept :
        m_ops{ops}
    {
    }

    void fail(
        const ice::sonic::Status& status,
        TF_Code code,
        std::string_view message
    ) const noexcept
    {
        ice::sonic::String text{m_ops.getStringOps()};
        text.create();
        text.copy(message.data(), message.size());
        status.set_status(code, text);
        text.destroy();
    }

    void fail_from(const ice::sonic::Status& status, const sycl::exception& error) const noexcept
    {
        fail(status, TF_INTERNAL, error.what());
    }

    void fail_unimplemented(const ice::sonic::Status& status, std::string_view slot) const noexcept
    {
        fail(status, TF_UNIMPLEMENTED, slot);
    }

    void copy_into(const ice::sonic::String& target, std::string_view source) const noexcept
    {
        target.copy(source.data(), source.size());
    }

    static sycl::async_handler make_async_handler(std::shared_ptr<std::string> sink)
    {
        return [sink = std::move(sink)](const sycl::exception_list& errors)
        {
            for (const auto& pending: errors) {
                try {
                    std::rethrow_exception(pending);
                } catch (const sycl::exception& error) {
                    if (sink->empty()) {
                        sink->assign(error.what());
                    }
                }
            }
        };
    }

    const SyclOpsTable& getOps() const noexcept
    {
        return m_ops;
    }

private:
    const SyclOpsTable& m_ops;
};

} // namespace aten_xpu
