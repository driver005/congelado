// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/status.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/status.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_StatusOps
{
public:
    static TF_StatusOps* create(void* ctx) noexcept
    {
        return static_cast<TF_StatusOps*>(ctx);
    }

    template<typename HandleT>
    static TF_StatusOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_StatusOps*>(handle->plugin_data);
    }

    virtual ~TF_StatusOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> delete_status() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_status(TF_Code code, const char* msg) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_payload(const char* key, const char* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    for_each_payload(TF_PayloadVisitor visitor, void* capture) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_status_from_io_error(int error_code, const char* context) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_code(TF_Code* out_code) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> message(const char** out_message) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_StatusOps* get_generic_vtable()
    {
        static TF_StatusOps vtable = {
            .struct_size = TF_STATUS_STRUCT_SIZE,
            .delete_status =
                [](TF_Status* s) noexcept
            {
                auto* self = TF_StatusOps::create(s);
                auto res = self->delete_status();
                if (!res) {
                    res.error().to_c(s);
                }
            },
            .set_status =
                [](TF_Status* s, TF_Code code, const char* msg) noexcept
            {
                auto* self = TF_StatusOps::create(s);
                auto res = self->set_status(code, msg);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_payload =
                [](TF_Status* s, const char* key, const char* value) noexcept
            {
                auto* self = TF_StatusOps::create(s);
                auto res = self->set_payload(key, value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each_payload =
                [](const TF_Status* s, TF_PayloadVisitor visitor, void* capture) noexcept
            {
                auto* self = TF_StatusOps::create(s);
                auto res = self->for_each_payload(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_status_from_io_error =
                [](TF_Status* s, int error_code, const char* context) noexcept
            {
                auto* self = TF_StatusOps::create(s);
                auto res = self->set_status_from_io_error(error_code, context);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_code =
                [](const TF_Status* s, TF_Code* out_code) noexcept
            {
                auto* self = TF_StatusOps::create(s);
                auto res = self->get_code(out_code);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .message =
                [](const TF_Status* s, const char** out_message) noexcept
            {
                auto* self = TF_StatusOps::create(s);
                auto res = self->message(out_message);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
