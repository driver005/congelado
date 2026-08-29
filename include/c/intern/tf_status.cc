// Minimal backend for intern/tf_status.h — this header (and every other header under
// include/c/) previously had zero implementation anywhere in this repo (declarations
// only). TF_Status threads through the TF_Generator_* ABI's TF_Generator_Create/WriteFile,
// so it's a genuine prerequisite for that backend to link at all, not an unrelated add-on.
//
// Exception contract: every extern "C" entry point is noexcept. The struct uses
// std::string/std::unordered_map (C++), so the allocating assignments are wrapped in leaf
// catches that degrade to TF_RESOURCE_EXHAUSTED / empty state instead of letting a C++
// exception escape through the C ABI. new_status() allocates with std::nothrow so a
// failed allocation returns nullptr instead of throwing bad_alloc at a C caller.

#include "c/intern/tf_status.h"

#include <cerrno>
#include <cstring>
#include <string>
#include <unordered_map>

struct TF_Status
{
    TF_Code code{TF_OK};
    std::string message;
    std::unordered_map<std::string, std::string> payloads;
};

extern "C"
{
    TF_Status* new_status(void) noexcept
    {
        try {
            return new TF_Status{};
        } catch (...) {
            // TF_Status's std::string/unordered_map members can throw bad_alloc during
            // construction — never let that cross the C ABI.
            return nullptr;
        }
    }

    void delete_status(TF_Status* status) noexcept
    {
        delete status;
    }

    void set_status(TF_Status* status, TF_Code code, const char* msg) noexcept
    {
        if (!status) {
            return;
        }
        status->code = code;
        try {
            status->message = msg ? msg : "";
        } catch (...) {
            status->code = TF_RESOURCE_EXHAUSTED;
            status->message.clear();
        }
        if (code == TF_OK) {
            status->payloads.clear();
        }
    }

    void set_payload(TF_Status* status, const char* key, const char* value) noexcept
    {
        if (!status || status->code == TF_OK) {
            return;
        }
        try {
            status->payloads[key ? key : ""] = value ? value : "";
        } catch (...) {
            status->code = TF_RESOURCE_EXHAUSTED;
            status->payloads.clear();
        }
    }

    void for_each_payload(const TF_Status* status, TF_PayloadVisitor visitor, void* capture) noexcept
    {
        if (!status || !visitor) {
            return;
        }
        for (const auto& [key, value]: status->payloads) {
            visitor(key.c_str(), key.size(), value.c_str(), value.size(), capture);
        }
    }

    void set_statusFromIOError(TF_Status* status, int error_code, const char* context) noexcept
    {
        if (!status) {
            return;
        }
        TF_Code code = TF_UNKNOWN;
        if (error_code == 0) {
            code = TF_OK;
        } else if (error_code == ENOENT) {
            code = TF_NOT_FOUND;
        } else if (error_code == EACCES || error_code == EPERM) {
            code = TF_PERMISSION_DENIED;
        } else if (error_code == EEXIST) {
            code = TF_ALREADY_EXISTS;
        } else if (error_code == EINVAL) {
            code = TF_INVALID_ARGUMENT;
        }
        status->code = code;
        try {
            status->message = context ? context : "";
        } catch (...) {
            status->code = TF_RESOURCE_EXHAUSTED;
            status->message.clear();
        }
    }

    TF_Code get_code(const TF_Status* status) noexcept
    {
        return status ? status->code : TF_OK;
    }

    const char* message(const TF_Status* status) noexcept
    {
        if (!status || status->code == TF_OK) {
            return "";
        }
        return status->message.c_str();
    }

} // extern "C"
