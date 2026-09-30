// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/status.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/status.h"

export module cc_ice_intern_builder:status;

import std;

export namespace ice::builder {

class Status
{
public:
    Status() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    Status(const Status&) = delete;
    Status& operator=(const Status&) = delete;

    static Status& from_handle(void* ctx) noexcept
    {
        return *static_cast<Status*>(ctx);
    }

    template<typename HandleT>
    static Status& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Status*>(handle->plugin_data);
    }

    virtual ~Status() = default;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> delete_status() noexcept = 0;
    virtual void set_status(TF_Code code, const ice::sonic::String& msg) noexcept = 0;
    virtual void
    set_payload(const ice::sonic::String& key, const ice::sonic::String& value) noexcept = 0;
    virtual void for_each_payload(TF_PayloadVisitor visitor, void* capture) noexcept = 0;
    virtual void
    set_status_from_io_error(int error_code, const ice::sonic::String& context) noexcept = 0;
    virtual void get_code(TF_Code* out_code) noexcept = 0;
    virtual void message(const ice::sonic::String& out_message) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_StatusOps{
            .struct_size = TF_STATUS_STRUCT_SIZE,
            .delete_status =
                [](TF_Status* s) noexcept
            {
                auto res = Status::from_handle(s).delete_status();
                if (!res) {
                    res.error().to_c(s);
                }
            },
            .set_status =
                [](TF_Status* s, TF_Code code, const TF_String* msg) noexcept
            {
                Status::from_handle(s).set_status(code, ice::sonic::String::wrap(msg));
            },
            .set_payload =
                [](TF_Status* s, const TF_String* key, const TF_String* value) noexcept
            {
                Status::from_handle(s).set_payload(
                    ice::sonic::String::wrap(key),
                    ice::sonic::String::wrap(value)
                );
            },
            .for_each_payload =
                [](const TF_Status* s, TF_PayloadVisitor visitor, void* capture) noexcept
            {
                Status::from_handle(s).for_each_payload(visitor, capture);
            },
            .set_status_from_io_error =
                [](TF_Status* s, int error_code, const TF_String* context) noexcept
            {
                Status::from_handle(s).set_status_from_io_error(
                    error_code,
                    ice::sonic::String::wrap(context)
                );
            },
            .get_code =
                [](const TF_Status* s, TF_Code* out_code) noexcept
            {
                Status::from_handle(s).get_code(out_code);
            },
            .message =
                [](const TF_Status* s, TF_String* out_message) noexcept
            {
                Status::from_handle(s).message(ice::sonic::String::wrap(out_message));
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
