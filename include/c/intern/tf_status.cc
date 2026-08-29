// Minimal backend for intern/tf_status.h — this header (and every other header under
// include/c/) previously had zero implementation anywhere in this repo (declarations
// only). TF_Status threads through the TF_Generator_* ABI's TF_Generator_Create/WriteFile,
// so it's a genuine prerequisite for that backend to link at all, not an unrelated add-on.

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
    TF_Status* new_status(void)
    {
        return new TF_Status{};
    }

    void delete_status(TF_Status* status)
    {
        delete status;
    }

    void set_status(TF_Status* status, TF_Code code, const char* msg)
    {
        if (!status) {
            return;
        }
        status->code = code;
        status->message = msg ? msg : "";
        if (code == TF_OK) {
            status->payloads.clear();
        }
    }

    void set_payload(TF_Status* status, const char* key, const char* value)
    {
        if (!status || status->code == TF_OK) {
            return;
        }
        status->payloads[key ? key : ""] = value ? value : "";
    }

    void for_each_payload(const TF_Status* status, TF_PayloadVisitor visitor, void* capture)
    {
        if (!status || !visitor) {
            return;
        }
        for (const auto& [key, value]: status->payloads) {
            visitor(key.c_str(), key.size(), value.c_str(), value.size(), capture);
        }
    }

    void set_statusFromIOError(TF_Status* status, int error_code, const char* context)
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
        status->message = context ? context : "";
    }

    TF_Code get_code(const TF_Status* status)
    {
        return status ? status->code : TF_OK;
    }

    const char* message(const TF_Status* status)
    {
        if (!status || status->code == TF_OK) {
            return "";
        }
        return status->message.c_str();
    }

} // extern "C"
