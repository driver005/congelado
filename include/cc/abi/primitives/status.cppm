module;

#include "c/intern/tf_status.h"

export module cc_abi_primitives:status;

import std;

export namespace ice {

enum class StatusCode
{
    Ok = 0,
    Cancelled = 1,
    Unknown = 2,
    InvalidArgument = 3,
    DeadlineExceeded = 4,
    NotFound = 5,
    AlreadyExists = 6,
    PermissionDenied = 7,
    ResourceExhausted = 8,
    FailedPrecondition = 9,
    Aborted = 10,
    OutOfRange = 11,
    Unimplemented = 12,
    Internal = 13,
    Unavailable = 14,
    DataLoss = 15,
    Unauthenticated = 16,
};

inline TF_Code status_code_to_c(StatusCode code) noexcept
{
    return static_cast<TF_Code>(code);
}

inline StatusCode status_code_from_c(TF_Code code) noexcept
{
    return static_cast<StatusCode>(code);
}

class Status
{
public:
    Status()
    {
        m_status = new_status();
    }

    Status(std::string message) :
        Status(StatusCode::Unknown, std::move(message))
    {
    }

    Status(StatusCode code, std::string message)
    {
        m_status = new_status();
        set_status(m_status, status_code_to_c(code), message.c_str());
    }

    Status(const Status& other)
    {
        m_status = new_status();
        set_status(m_status, get_code(other.m_status), ::message(other.m_status));
    }

    Status& operator=(const Status& other)
    {
        if (this != &other) {
            set_status(m_status, get_code(other.m_status), ::message(other.m_status));
        }
        return *this;
    }

    Status(Status&& other) noexcept
    {
        m_status = other.m_status;
        other.m_status = nullptr;
    }

    Status& operator=(Status&& other) noexcept
    {
        if (this != &other) {
            if (m_status) {
                delete_status(m_status);
            }
            m_status = other.m_status;
            other.m_status = nullptr;
        }
        return *this;
    }

    ~Status()
    {
        if (m_status) {
            delete_status(m_status);
        }
    }

    bool ok() const
    {
        return get_code(m_status) == TF_OK;
    }

    StatusCode code() const
    {
        return status_code_from_c(get_code(m_status));
    }

    std::string message() const
    {
        const char* msg = ::message(m_status);
        return msg ? std::string(msg) : std::string();
    }

    void set_code(StatusCode code)
    {
        set_status(m_status, status_code_to_c(code), ::message(m_status));
    }

    void set_message(std::string message)
    {
        set_status(m_status, get_code(m_status), message.c_str());
    }

    void to_c(TF_Status* out_status) const
    {
        if (out_status && m_status) {
            set_status(out_status, get_code(m_status), ::message(m_status));
        }
    }

    static Status create(TF_Status* s)
    {
        Status status;
        if (s) {
            set_status(status.m_status, get_code(s), ::message(s));
        }
        return status;
    }

    TF_Status* get_handle() const
    {
        return m_status;
    }

private:
    TF_Status* m_status;
};

} // namespace ice
