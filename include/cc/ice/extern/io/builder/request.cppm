// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/request.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/request.h"

export module cc_ice_extern_io_builder:request;

import std;

export namespace ice::builder {

class TF_RequestOps
{
public:
    TF_RequestOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_RequestOps(const TF_RequestOps&) = delete;
    TF_RequestOps& operator=(const TF_RequestOps&) = delete;

    static TF_RequestOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_RequestOps*>(ctx);
    }

    template<typename HandleT>
    static TF_RequestOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_RequestOps*>(handle->plugin_data);
    }

    virtual ~TF_RequestOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_method(const ice::sonic::String& method) noexcept = 0;
    virtual void get_method(const ice::sonic::String& out_method) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_path(const ice::sonic::String& path) noexcept = 0;
    virtual void get_path(const ice::sonic::String& out_path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_scheme(const ice::sonic::String& scheme) noexcept = 0;
    virtual void get_scheme(const ice::sonic::String& out_scheme) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_authority(const ice::sonic::String& authority) noexcept = 0;
    virtual void get_authority(const ice::sonic::String& out_authority) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_header(const ice::sonic::String& name, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    add_header(const ice::sonic::String& name, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    remove_header(const ice::sonic::String& name) noexcept = 0;
    virtual void
    find_header(const ice::sonic::String& name, const TF_String** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> clear_headers() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_headers(const ice::sonic::TF_MapOps& out_headers) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_query_param(const ice::sonic::String& name, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_query_params(const ice::sonic::TF_MapOps& out_params) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_body(const void* data, size_t length) noexcept = 0;
    virtual void get_body(const void** out_data, size_t* out_length) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_content_type(const ice::sonic::String& content_type) noexcept = 0;
    virtual void get_content_type(const ice::sonic::String& out_content_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_accept(const ice::sonic::String& accept) noexcept = 0;
    virtual void get_accept(const ice::sonic::String& out_accept) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_user_agent(const ice::sonic::String& user_agent) noexcept = 0;
    virtual void get_user_agent(const ice::sonic::String& out_user_agent) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_bearer_auth(const ice::sonic::String& token) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> set_basic_auth(
        const ice::sonic::String& username,
        const ice::sonic::String& password
    ) noexcept = 0;
    virtual void get_authorization(const ice::sonic::String& out_authorization) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_addr(const ice::sonic::String& addr) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_no_decompress(int enabled) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_timeout(int64_t timeout_ms) noexcept = 0;
    virtual void get_timeout(int64_t* out_timeout_ms) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set_stream_id(uint32_t stream_id) noexcept = 0;
    virtual void get_stream_id(uint32_t* out_stream_id) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_RequestOps{
            .struct_size = TF_REQUEST_STRUCT_SIZE,
            .destroy =
                [](TF_Request* request) noexcept
            {
                TF_RequestOps::from_handle(request).destroy();
            },
            .get_name =
                [](TF_Request* request, TF_String* out_name) noexcept
            {
                TF_RequestOps::from_handle(request).get_name(ice::sonic::String::wrap(out_name));
            },
            .set_method =
                [](TF_Request* request, const TF_String* method, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_method(
                    ice::sonic::String::wrap(method)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_method =
                [](TF_Request* request, TF_String* out_method) noexcept
            {
                TF_RequestOps::from_handle(request).get_method(
                    ice::sonic::String::wrap(out_method)
                );
            },
            .set_path =
                [](TF_Request* request, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto res =
                    TF_RequestOps::from_handle(request).set_path(ice::sonic::String::wrap(path));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_path =
                [](TF_Request* request, TF_String* out_path) noexcept
            {
                TF_RequestOps::from_handle(request).get_path(ice::sonic::String::wrap(out_path));
            },
            .set_scheme =
                [](TF_Request* request, const TF_String* scheme, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_scheme(
                    ice::sonic::String::wrap(scheme)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_scheme =
                [](TF_Request* request, TF_String* out_scheme) noexcept
            {
                TF_RequestOps::from_handle(request).get_scheme(
                    ice::sonic::String::wrap(out_scheme)
                );
            },
            .set_authority =
                [](TF_Request* request, const TF_String* authority, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_authority(
                    ice::sonic::String::wrap(authority)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_authority =
                [](TF_Request* request, TF_String* out_authority) noexcept
            {
                TF_RequestOps::from_handle(request).get_authority(
                    ice::sonic::String::wrap(out_authority)
                );
            },
            .set_header =
                [](TF_Request* request,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_header(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_header =
                [](TF_Request* request,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).add_header(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .remove_header =
                [](TF_Request* request, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).remove_header(
                    ice::sonic::String::wrap(name)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .find_header =
                [](TF_Request* request, const TF_String* name, const TF_String** out_value) noexcept
            {
                TF_RequestOps::from_handle(request).find_header(
                    ice::sonic::String::wrap(name),
                    out_value
                );
            },
            .clear_headers =
                [](TF_Request* request, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).clear_headers();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_headers =
                [](TF_Request* request, TF_Map* out_headers, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).get_headers(
                    ice::sonic::TF_MapOps::wrap(out_headers)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_query_param =
                [](TF_Request* request,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_query_param(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_query_params =
                [](TF_Request* request, TF_Map* out_params, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).get_query_params(
                    ice::sonic::TF_MapOps::wrap(out_params)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_body =
                [](TF_Request* request,
                   const void* data,
                   size_t length,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_body(data, length);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_body =
                [](TF_Request* request, const void** out_data, size_t* out_length) noexcept
            {
                TF_RequestOps::from_handle(request).get_body(out_data, out_length);
            },
            .set_content_type =
                [](TF_Request* request,
                   const TF_String* content_type,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_content_type(
                    ice::sonic::String::wrap(content_type)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_content_type =
                [](TF_Request* request, TF_String* out_content_type) noexcept
            {
                TF_RequestOps::from_handle(request).get_content_type(
                    ice::sonic::String::wrap(out_content_type)
                );
            },
            .set_accept =
                [](TF_Request* request, const TF_String* accept, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_accept(
                    ice::sonic::String::wrap(accept)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_accept =
                [](TF_Request* request, TF_String* out_accept) noexcept
            {
                TF_RequestOps::from_handle(request).get_accept(
                    ice::sonic::String::wrap(out_accept)
                );
            },
            .set_user_agent =
                [](TF_Request* request, const TF_String* user_agent, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_user_agent(
                    ice::sonic::String::wrap(user_agent)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_user_agent =
                [](TF_Request* request, TF_String* out_user_agent) noexcept
            {
                TF_RequestOps::from_handle(request).get_user_agent(
                    ice::sonic::String::wrap(out_user_agent)
                );
            },
            .set_bearer_auth =
                [](TF_Request* request, const TF_String* token, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_bearer_auth(
                    ice::sonic::String::wrap(token)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_basic_auth =
                [](TF_Request* request,
                   const TF_String* username,
                   const TF_String* password,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_basic_auth(
                    ice::sonic::String::wrap(username),
                    ice::sonic::String::wrap(password)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_authorization =
                [](TF_Request* request, TF_String* out_authorization) noexcept
            {
                TF_RequestOps::from_handle(request).get_authorization(
                    ice::sonic::String::wrap(out_authorization)
                );
            },
            .set_addr =
                [](TF_Request* request, const TF_String* addr, TF_Status* out_status) noexcept
            {
                auto res =
                    TF_RequestOps::from_handle(request).set_addr(ice::sonic::String::wrap(addr));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_no_decompress =
                [](TF_Request* request, int enabled, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_no_decompress(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_timeout =
                [](TF_Request* request, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_timeout(timeout_ms);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_timeout =
                [](TF_Request* request, int64_t* out_timeout_ms) noexcept
            {
                TF_RequestOps::from_handle(request).get_timeout(out_timeout_ms);
            },
            .set_stream_id =
                [](TF_Request* request, uint32_t stream_id, TF_Status* out_status) noexcept
            {
                auto res = TF_RequestOps::from_handle(request).set_stream_id(stream_id);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stream_id =
                [](TF_Request* request, uint32_t* out_stream_id) noexcept
            {
                TF_RequestOps::from_handle(request).get_stream_id(out_stream_id);
            },

        };
    }

    const ::TF_RequestOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Request& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_RequestOps m_vtable;
    TF_Request m_handle;
};

} // namespace ice::builder
