module;

#include "c/intern/tf_status.h"

export module cc_abi_value:status;

import std;

export namespace ice {

enum class StatusCode {
    Ok = 0,
    Cancelled = 1,
    Unknown = 2,
    InvalidArgument = 3,
    DeadlineExceeded = 4,
    NotFound = 5,
    AlreadyExists = 6,
    PermissionDenied = 7,
    Unauthenticated = 16,
    ResourceExhausted = 8,
    FailedPrecondition = 9,
    Aborted = 10,
    OutOfRange = 11,
    Unimplemented = 12,
    Internal = 13,
    Unavailable = 14,
    DataLoss = 15,
};

class Status {
  public:
    Status() : m_code{StatusCode::Ok} {}
    Status(StatusCode code, std::string message) : m_code{code}, m_message{std::move(message)} {}

    bool ok() const { return m_code == StatusCode::Ok; }
    StatusCode get_code() const { return m_code; }
    const std::string &get_message() const { return m_message; }

    void set_code(StatusCode code) { m_code = code; }
    void set_message(std::string message) { m_message = std::move(message); }

    static Status from_c(TF_Status *s) {

        if (!s)
            return Status{};
        TF_Code code = TF_GetCode(s);
        if (code == TF_OK)
            return Status{};
        return Status(static_cast<StatusCode>(code), TF_Message(s));

    }

  private:
    StatusCode m_code;
    std::string m_message;
};

} // namespace ice
