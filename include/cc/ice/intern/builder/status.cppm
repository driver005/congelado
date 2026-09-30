// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/status.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/status.h"

export module cc_ice_intern_builder:status;

import std;

export namespace ice::builder {

class TF_StatusOps
{
public:
    TF_StatusOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_StatusOps(const TF_StatusOps&) = delete;
    TF_StatusOps& operator=(const TF_StatusOps&) = delete;

    static TF_StatusOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_StatusOps*>(ctx);
    }

    template<typename HandleT>
    static TF_StatusOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_StatusOps*>(handle->plugin_data);
    }

    virtual ~TF_StatusOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> delete_status() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_status(TF_Code code, const ice::sonic::String& msg) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_payload(const ice::sonic::String& key, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    for_each_payload(TF_PayloadVisitor visitor, void* capture) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_status_from_io_error(int error_code, const ice::sonic::String& context) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_code(TF_Code* out_code) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    message(const ice::sonic::String& out_message) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_StatusOps{
            .struct_size = TF_STATUS_STRUCT_SIZE,
            .delete_status =
                [](TF_Status* s) noexcept
            {
                auto res = TF_StatusOps::from_handle(s).delete_status();
                if (!res) {
                    res.error().to_c(s);
                }
            },
            .set_status =
                [](TF_Status* s, TF_Code code, const TF_String* msg) noexcept
            {
                auto res =
                    TF_StatusOps::from_handle(s).set_status(code, ice::sonic::String::wrap(msg));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_payload =
                [](TF_Status* s, const TF_String* key, const TF_String* value) noexcept
            {
                auto res = TF_StatusOps::from_handle(s).set_payload(
                    ice::sonic::String::wrap(key),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .for_each_payload =
                [](const TF_Status* s, TF_PayloadVisitor visitor, void* capture) noexcept
            {
                auto res = TF_StatusOps::from_handle(s).for_each_payload(visitor, capture);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_status_from_io_error =
                [](TF_Status* s, int error_code, const TF_String* context) noexcept
            {
                auto res = TF_StatusOps::from_handle(s).set_status_from_io_error(
                    error_code,
                    ice::sonic::String::wrap(context)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_code =
                [](const TF_Status* s, TF_Code* out_code) noexcept
            {
                auto res = TF_StatusOps::from_handle(s).get_code(out_code);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .message =
                [](const TF_Status* s, TF_String* out_message) noexcept
            {
                auto res =
                    TF_StatusOps::from_handle(s).message(ice::sonic::String::wrap(out_message));
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_StatusOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Status& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_StatusOps m_vtable;
    TF_Status m_handle;
};

} // namespace ice::builder
