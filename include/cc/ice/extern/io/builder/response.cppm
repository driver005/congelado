// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/response.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/response.h"

export module cc_ice_extern_io_builder:response;

import std;

export namespace ice::builder {

class TF_ResponseOps
{
public:
    TF_ResponseOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_ResponseOps(const TF_ResponseOps&) = delete;
    TF_ResponseOps& operator=(const TF_ResponseOps&) = delete;

    static TF_ResponseOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ResponseOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ResponseOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ResponseOps*>(handle->plugin_data);
    }

    virtual ~TF_ResponseOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_status(int32_t status_code) noexcept = 0;
    virtual void get_status(int32_t* out_status_code) noexcept = 0;
    virtual void get_status_text(const ice::sonic::String& out_status_text) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_header(const ice::sonic::String& name, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    add_header(const ice::sonic::String& name, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    remove_header(const ice::sonic::String& name) noexcept = 0;
    virtual void
    find_header(const ice::sonic::String& name, const TF_String** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_headers(const ice::sonic::TF_MapOps& out_headers) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_body(const void* data, size_t length) noexcept = 0;
    virtual void get_body(const void** out_data, size_t* out_length) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_keep_alive(int enabled) noexcept = 0;
    virtual void is_keep_alive(int* out_keep_alive) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> set_cookie(
        const ice::sonic::String& name,
        const ice::sonic::String& value,
        const ice::sonic::TF_MapOps& attributes
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_set_cookies(const ice::sonic::TF_VectorOps& out_cookies) noexcept = 0;
    virtual void get_content_type(const ice::sonic::String& out_content_type) noexcept = 0;
    virtual void get_content_length(int64_t* out_length) noexcept = 0;
    virtual void get_location(const ice::sonic::String& out_location) noexcept = 0;
    virtual void get_etag(const ice::sonic::String& out_etag) noexcept = 0;
    virtual void get_date(const ice::sonic::String& out_date) noexcept = 0;
    virtual void get_server(const ice::sonic::String& out_server) noexcept = 0;
    virtual void get_cache_control(const ice::sonic::String& out_cache_control) noexcept = 0;
    virtual void get_last_modified(const ice::sonic::String& out_last_modified) noexcept = 0;
    virtual void is_informational(int* out_result) noexcept = 0;
    virtual void is_success(int* out_result) noexcept = 0;
    virtual void is_redirection(int* out_result) noexcept = 0;
    virtual void is_client_error(int* out_result) noexcept = 0;
    virtual void is_server_error(int* out_result) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ResponseOps{
            .struct_size = TF_RESPONSE_STRUCT_SIZE,
            .destroy =
                [](TF_Response* response) noexcept
            {
                TF_ResponseOps::from_handle(response).destroy();
            },
            .get_name =
                [](TF_Response* response, TF_String* out_name) noexcept
            {
                TF_ResponseOps::from_handle(response).get_name(ice::sonic::String::wrap(out_name));
            },
            .set_status =
                [](TF_Response* response, int32_t status_code, TF_Status* out_status) noexcept
            {
                auto res = TF_ResponseOps::from_handle(response).set_status(status_code);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_status =
                [](TF_Response* response, int32_t* out_status_code) noexcept
            {
                TF_ResponseOps::from_handle(response).get_status(out_status_code);
            },
            .get_status_text =
                [](TF_Response* response, TF_String* out_status_text) noexcept
            {
                TF_ResponseOps::from_handle(response).get_status_text(
                    ice::sonic::String::wrap(out_status_text)
                );
            },
            .set_header =
                [](TF_Response* response,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ResponseOps::from_handle(response).set_header(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_header =
                [](TF_Response* response,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ResponseOps::from_handle(response).add_header(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .remove_header =
                [](TF_Response* response, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto res = TF_ResponseOps::from_handle(response).remove_header(
                    ice::sonic::String::wrap(name)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .find_header =
                [](TF_Response* response,
                   const TF_String* name,
                   const TF_String** out_value) noexcept
            {
                TF_ResponseOps::from_handle(response).find_header(
                    ice::sonic::String::wrap(name),
                    out_value
                );
            },
            .get_headers =
                [](TF_Response* response, TF_Map* out_headers, TF_Status* out_status) noexcept
            {
                auto res = TF_ResponseOps::from_handle(response).get_headers(
                    ice::sonic::TF_MapOps::wrap(out_headers)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_body =
                [](TF_Response* response,
                   const void* data,
                   size_t length,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ResponseOps::from_handle(response).set_body(data, length);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_body =
                [](TF_Response* response, const void** out_data, size_t* out_length) noexcept
            {
                TF_ResponseOps::from_handle(response).get_body(out_data, out_length);
            },
            .set_keep_alive =
                [](TF_Response* response, int enabled, TF_Status* out_status) noexcept
            {
                auto res = TF_ResponseOps::from_handle(response).set_keep_alive(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .is_keep_alive =
                [](TF_Response* response, int* out_keep_alive) noexcept
            {
                TF_ResponseOps::from_handle(response).is_keep_alive(out_keep_alive);
            },
            .set_cookie =
                [](TF_Response* response,
                   const TF_String* name,
                   const TF_String* value,
                   const TF_Map* attributes,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ResponseOps::from_handle(response).set_cookie(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value),
                    ice::sonic::TF_MapOps::wrap(attributes)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_set_cookies =
                [](TF_Response* response, TF_Vector* out_cookies, TF_Status* out_status) noexcept
            {
                auto res = TF_ResponseOps::from_handle(response).get_set_cookies(
                    ice::sonic::TF_VectorOps::wrap(out_cookies)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_content_type =
                [](TF_Response* response, TF_String* out_content_type) noexcept
            {
                TF_ResponseOps::from_handle(response).get_content_type(
                    ice::sonic::String::wrap(out_content_type)
                );
            },
            .get_content_length =
                [](TF_Response* response, int64_t* out_length) noexcept
            {
                TF_ResponseOps::from_handle(response).get_content_length(out_length);
            },
            .get_location =
                [](TF_Response* response, TF_String* out_location) noexcept
            {
                TF_ResponseOps::from_handle(response).get_location(
                    ice::sonic::String::wrap(out_location)
                );
            },
            .get_etag =
                [](TF_Response* response, TF_String* out_etag) noexcept
            {
                TF_ResponseOps::from_handle(response).get_etag(ice::sonic::String::wrap(out_etag));
            },
            .get_date =
                [](TF_Response* response, TF_String* out_date) noexcept
            {
                TF_ResponseOps::from_handle(response).get_date(ice::sonic::String::wrap(out_date));
            },
            .get_server =
                [](TF_Response* response, TF_String* out_server) noexcept
            {
                TF_ResponseOps::from_handle(response).get_server(
                    ice::sonic::String::wrap(out_server)
                );
            },
            .get_cache_control =
                [](TF_Response* response, TF_String* out_cache_control) noexcept
            {
                TF_ResponseOps::from_handle(response).get_cache_control(
                    ice::sonic::String::wrap(out_cache_control)
                );
            },
            .get_last_modified =
                [](TF_Response* response, TF_String* out_last_modified) noexcept
            {
                TF_ResponseOps::from_handle(response).get_last_modified(
                    ice::sonic::String::wrap(out_last_modified)
                );
            },
            .is_informational =
                [](TF_Response* response, int* out_result) noexcept
            {
                TF_ResponseOps::from_handle(response).is_informational(out_result);
            },
            .is_success =
                [](TF_Response* response, int* out_result) noexcept
            {
                TF_ResponseOps::from_handle(response).is_success(out_result);
            },
            .is_redirection =
                [](TF_Response* response, int* out_result) noexcept
            {
                TF_ResponseOps::from_handle(response).is_redirection(out_result);
            },
            .is_client_error =
                [](TF_Response* response, int* out_result) noexcept
            {
                TF_ResponseOps::from_handle(response).is_client_error(out_result);
            },
            .is_server_error =
                [](TF_Response* response, int* out_result) noexcept
            {
                TF_ResponseOps::from_handle(response).is_server_error(out_result);
            },

        };
    }

    const ::TF_ResponseOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Response& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ResponseOps m_vtable;
    TF_Response m_handle;
};

} // namespace ice::builder
