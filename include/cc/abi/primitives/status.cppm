module;

#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

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

inline TF_Code status_code_to_c(StatusCode code) noexcept {
    return static_cast<TF_Code>(code);
}
inline StatusCode status_code_from_c(TF_Code code) noexcept {
    return static_cast<StatusCode>(code);
}

class Status
{
private:
    struct RuntimeState {
        TF_StatusOps* ops;
        void* host_context;
    };

    static RuntimeState state() {
        RuntimeState s;
        TF_InitStatus(&s.ops, &s.host_context);
        return s;
    }

public:
    Status()
    {
        auto rs = state();
        m_status = rs.ops->TF_NewStatus(rs.host_context);
    }

    Status(std::string message) : Status(StatusCode::Unknown, message) {}
    Status(StatusCode code, std::string message)
    {
        auto rs = state();
        m_status = rs.ops->TF_NewStatus(rs.host_context);
        TF_String msg;
        TF_TStringOps* tops;
        void* thost;
        TF_InitTString(&tops, &thost);
        tops->TF_StringInit(thost, &msg);
        tops->TF_StringAssignView(thost, &msg, message.data(), message.size());
        
        rs.ops->TF_SetStatus(rs.host_context, m_status, status_code_to_c(code), msg);
        
        tops->TF_StringDealloc(thost, &msg);
    }

    Status(const Status& other)
    {
        auto rs = state();
        m_status = rs.ops->TF_NewStatus(rs.host_context);
        auto code = rs.ops->TF_GetCode(rs.host_context, other.m_status);
        auto msg = rs.ops->TF_Message(rs.host_context, other.m_status);
        rs.ops->TF_SetStatus(rs.host_context, m_status, code, msg);
    }

    Status& operator=(const Status& other)
    {
        if (this != &other) {
            auto rs = state();
            auto code = rs.ops->TF_GetCode(rs.host_context, other.m_status);
            auto msg = rs.ops->TF_Message(rs.host_context, other.m_status);
            rs.ops->TF_SetStatus(rs.host_context, m_status, code, msg);
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
            auto rs = state();
            if (m_status) {
                rs.ops->TF_DeleteStatus(rs.host_context, m_status);
            }
            m_status = other.m_status;
            other.m_status = nullptr;
        }
        return *this;
    }

    ~Status()
    {
        if (m_status) {
            auto rs = state();
            rs.ops->TF_DeleteStatus(rs.host_context, m_status);
        }
    }

    bool ok() const
    {
        auto rs = state();
        return rs.ops->TF_GetCode(rs.host_context, m_status) == TF_OK;
    }

    StatusCode code() const
    {
        auto rs = state();
        return status_code_from_c(rs.ops->TF_GetCode(rs.host_context, m_status));
    }

    std::string message() const
    {
        auto rs = state();
        TF_String msg = rs.ops->TF_Message(rs.host_context, m_status);
        TF_TStringOps* tops;
        void* thost;
        TF_InitTString(&tops, &thost);
        const char* data = tops->TF_StringGetDataPointer(thost, &msg);
        size_t size = tops->TF_StringGetSize(thost, &msg);
        if (data && size > 0) {
            return std::string(data, size);
        }
        return "";
    }

    void set_code(StatusCode code)
    {
        auto rs = state();
        TF_String msg = rs.ops->TF_Message(rs.host_context, m_status);
        rs.ops->TF_SetStatus(rs.host_context, m_status, status_code_to_c(code), msg);
    }

    void set_message(std::string message)
    {
        auto rs = state();
        auto code = rs.ops->TF_GetCode(rs.host_context, m_status);
        
        TF_String msg;
        TF_TStringOps* tops;
        void* thost;
        TF_InitTString(&tops, &thost);
        tops->TF_StringInit(thost, &msg);
        tops->TF_StringAssignView(thost, &msg, message.data(), message.size());
        
        rs.ops->TF_SetStatus(rs.host_context, m_status, code, msg);
        
        tops->TF_StringDealloc(thost, &msg);
    }

    
void to_c(TF_Status* out_status) const
    {
        if (out_status && m_status) {
            auto rs = state();
            auto code = rs.ops->TF_GetCode(rs.host_context, m_status);
            auto msg = rs.ops->TF_Message(rs.host_context, m_status);
            rs.ops->TF_SetStatus(rs.host_context, out_status, code, msg);
        }
    }

    static Status create(TF_Status* s)
    {
        Status status;
        if (s) {
            auto rs = state();
            auto code = rs.ops->TF_GetCode(rs.host_context, s);
            auto msg = rs.ops->TF_Message(rs.host_context, s);
            rs.ops->TF_SetStatus(rs.host_context, status.m_status, code, msg);
        }
        return status;
    }
    
    TF_Status* get_handle() const { return m_status; }

private:
    TF_Status* m_status;
};

} // namespace ice
