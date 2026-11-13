module;

#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <cerrno>

export module aten_xpu_intern:status_store;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :string;

export namespace aten_xpu {

// A host status: code, message and a sorted payload map.
class Status : public ice::builder::Status
{
public:
    explicit Status(const ::TF_StringOps* string_ops) noexcept :
        ice::builder::Status{string_ops}
    {
    }

    ~Status() override = default;
    Status(const Status&) = delete;
    Status& operator=(const Status&) = delete;
    Status(Status&&) = delete;
    Status& operator=(Status&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void delete_status() noexcept override
    {
        delete this;
    }

    void set_status(ice::sonic::Code code, const ice::sonic::String& msg) noexcept override
    {
        m_code = code;
        m_message.assign(String::view_of(msg));
    }

    void set_payload(
        const ice::sonic::String& key,
        const ice::sonic::String& value
    ) noexcept override
    {
        m_payloads.insert_or_assign(
            std::string{String::view_of(key)},
            std::string{String::view_of(value)}
        );
    }

    void for_each_payload(TF_PayloadVisitor visitor, void* capture) noexcept override
    {
        for (const auto& [key, value]: m_payloads) {
            visitor(key.data(), key.size(), value.data(), value.size(), capture);
        }
    }

    void set_status_from_io_error(
        int error_code,
        const ice::sonic::String& context
    ) noexcept override
    {
        m_code = code_for_errno(error_code);
        m_message.assign(String::view_of(context));
        m_message += ": ";
        m_message += std::error_code{error_code, std::generic_category()}.message();
    }

    void get_code(ice::sonic::Code& out_code) noexcept override
    {
        out_code = m_code;
    }

    void message(const ice::sonic::String& out_message) noexcept override
    {
        out_message.copy(m_message);
    }

private:
    static ice::sonic::Code code_for_errno(int error_code) noexcept
    {
        switch (error_code) {
            case 0:
                return ice::sonic::Code::k_ok;
            case ENOENT:
                return ice::sonic::Code::k_not_found;
            case EEXIST:
                return ice::sonic::Code::k_already_exists;
            case EACCES:
            case EPERM:
            case EROFS:
                return ice::sonic::Code::k_permission_denied;
            case EINVAL:
            case EBADF:
            case ENAMETOOLONG:
                return ice::sonic::Code::k_invalid_argument;
            case ENOSPC:
            case ENOMEM:
            case EMFILE:
            case ENFILE:
            case EDQUOT:
                return ice::sonic::Code::k_resource_exhausted;
            case ENOTDIR:
            case EISDIR:
            case ENOTEMPTY:
            case EBUSY:
                return ice::sonic::Code::k_failed_precondition;
            case ERANGE:
            case EOVERFLOW:
                return ice::sonic::Code::k_out_of_range;
            case ENOSYS:
            case ENOTSUP:
                return ice::sonic::Code::k_unimplemented;
            case EINTR:
            case EAGAIN:
            case ECONNRESET:
            case ETIMEDOUT:
                return ice::sonic::Code::k_unavailable;
            case ECANCELED:
                return ice::sonic::Code::k_cancelled;
            case EIO:
                return ice::sonic::Code::k_data_loss;
            default:
                return ice::sonic::Code::k_unknown;
        }
    }

    ice::sonic::Code m_code = ice::sonic::Code::k_ok;
    std::string m_message;
    std::map<std::string, std::string> m_payloads;
};

} // namespace aten_xpu
