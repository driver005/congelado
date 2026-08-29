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

// Status — RAII owner of a C TF_Status handle (unique_ptr + delete_status), so ownership,
// copy, move and moved-from state are all handled by the standard library instead of
// hand-rolled new/delete + null checks. Every member is noexcept: the underlying
// set_status/get_code/message C functions are themselves noexcept (see tf_status.cc), so no
// exception can cross the ABI through this class.
class Status
{
public:
    Status() noexcept :
        m_status{new_status(), &delete_status}
    {
    }

    Status(std::string_view message) noexcept :
        Status(StatusCode::Unknown, message)
    {
    }

    Status(StatusCode code, std::string_view message) noexcept :
        m_status{new_status(), &delete_status}
    {
        if (m_status) {
            set_status(m_status.get(), status_code_to_c(code), message.data());
        }
    }

    Status(const Status& other) noexcept :
        m_status{new_status(), &delete_status}
    {
        if (m_status && other.m_status) {
            set_status(m_status.get(), get_code(other.m_status.get()), ::message(other.m_status.get()));
        }
    }

    Status& operator=(const Status& other) noexcept
    {
        if (this != &other) {
            // Works even when *this is moved-from: m_status is a unique_ptr that stays null
            // and reset() simply adopts the fresh copy.
            m_status.reset(other.m_status ? new_status() : nullptr);
            if (m_status && other.m_status) {
                set_status(m_status.get(), get_code(other.m_status.get()), ::message(other.m_status.get()));
            }
        }
        return *this;
    }

    Status(Status&& other) noexcept = default;
    Status& operator=(Status&& other) noexcept = default;

    ~Status() noexcept = default;

    bool ok() const noexcept
    {
        return m_status && get_code(m_status.get()) == TF_OK;
    }

    StatusCode code() const noexcept
    {
        return m_status ? status_code_from_c(get_code(m_status.get())) : StatusCode::Ok;
    }

    // Non-allocating read-only view; valid until the next mutation of this Status.
    std::string_view message() const noexcept
    {
        const char* msg = m_status ? ::message(m_status.get()) : nullptr;
        return msg ? std::string_view{msg} : std::string_view{};
    }

    void set_code(StatusCode code) noexcept
    {
        if (m_status) {
            set_status(m_status.get(), status_code_to_c(code), ::message(m_status.get()));
        }
    }

    void set_message(std::string_view message) noexcept
    {
        if (m_status) {
            set_status(m_status.get(), get_code(m_status.get()), message.data());
        }
    }

    void to_c(TF_Status* out_status) const noexcept
    {
        if (out_status && m_status) {
            set_status(out_status, get_code(m_status.get()), ::message(m_status.get()));
        }
    }

    static Status create(TF_Status* s) noexcept
    {
        Status status;
        if (s) {
            set_status(status.m_status.get(), get_code(s), ::message(s));
        }
        return status;
    }

    // Underlying handle — pass directly to the C ABI.
    TF_Status* get_handle() noexcept
    {
        return m_status.get();
    }

    const TF_Status* get_handle() const noexcept
    {
        return m_status.get();
    }

private:
    std::unique_ptr<TF_Status, void (*)(TF_Status*)> m_status;
};

} // namespace ice
