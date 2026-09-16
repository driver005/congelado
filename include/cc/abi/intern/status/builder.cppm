// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/status/status.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/status/status.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_status;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Status
{
public:
    static Status* create(void* ctx) noexcept
    {
        return static_cast<Status*>(ctx);
    }

    template<typename HandleT>
    static Status* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Status*>(handle);
    }

    virtual ~Status() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_status() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> delete_status() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_status(TF_Code code, const char* msg) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_payload(const char* key, const char* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    for_each_payload(TF_PayloadVisitor visitor, void* capture) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_status_from_io_error(int error_code, const char* context) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_code() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> message() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Status* get_generic_vtable()
    {
        static TF_Status vtable = {
            .struct_size = TF_STATUS_STRUCT_SIZE,
            .new_status =
                [](void* plugin_context) noexcept
            {
                auto* self = Status::create(plugin_context);
                auto res = self->new_status();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_status =
                [](TF_Status_Handle* s) noexcept
            {
                auto* self = Status::create(s);
                auto res = self->delete_status();
                if (!res) {
                    res.error().to_c(s);
                }
            },
            .set_status =
                [](TF_Status_Handle* s, TF_Code code, const char* msg) noexcept
            {
                auto* self = Status::create(s);
                auto res = self->set_status(code, msg);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_payload =
                [](TF_Status_Handle* s, const char* key, const char* value) noexcept
            {
                auto* self = Status::create(s);
                auto res = self->set_payload(key, value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each_payload =
                [](const TF_Status_Handle* s, TF_PayloadVisitor visitor, void* capture) noexcept
            {
                auto* self = Status::create(s);
                auto res = self->for_each_payload(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_status_from_io_error =
                [](TF_Status_Handle* s, int error_code, const char* context) noexcept
            {
                auto* self = Status::create(s);
                auto res = self->set_status_from_io_error(error_code, context);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_code =
                [](const TF_Status_Handle* s) noexcept
            {
                auto* self = Status::create(s);
                auto res = self->get_code();
                if (!res) {
                    res.error().to_c(s);
                }
            },
            .message =
                [](const TF_Status_Handle* s) noexcept
            {
                auto* self = Status::create(s);
                auto res = self->message();
                if (!res) {
                    res.error().to_c(s);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
