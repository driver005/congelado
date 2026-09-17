// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/request.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/request.h"

export module cc_abi_builder_io;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_RequestOps
{
public:
    static TF_RequestOps* create(void* ctx) noexcept
    {
        return static_cast<TF_RequestOps*>(ctx);
    }

    template<typename HandleT>
    static TF_RequestOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_RequestOps*>(handle->plugin_data);
    }

    virtual ~TF_RequestOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_method(const ice::sonic::TF_StringOps& method) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_method(const ice::sonic::TF_StringOps& out_method) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_path(const ice::sonic::TF_StringOps& path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_path(const ice::sonic::TF_StringOps& out_path) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_scheme(const ice::sonic::TF_StringOps& scheme) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_scheme(const ice::sonic::TF_StringOps& out_scheme) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_authority(const ice::sonic::TF_StringOps& authority) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_authority(const ice::sonic::TF_StringOps& out_authority) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_header(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_StringOps& value
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> add_header(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_StringOps& value
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    remove_header(const ice::sonic::TF_StringOps& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    find_header(const ice::sonic::TF_StringOps& name, const TF_String** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> clear_headers() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_headers(const ice::sonic::TF_MapOps& out_headers) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_query_param(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_StringOps& value
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_query_params(const ice::sonic::TF_MapOps& out_params) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_body(const void* data, size_t length) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_body(const void** out_data, size_t* out_length) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_content_type(const ice::sonic::TF_StringOps& content_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_content_type(const ice::sonic::TF_StringOps& out_content_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_accept(const ice::sonic::TF_StringOps& accept) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_accept(const ice::sonic::TF_StringOps& out_accept) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_user_agent(const ice::sonic::TF_StringOps& user_agent) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_user_agent(const ice::sonic::TF_StringOps& out_user_agent) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_bearer_auth(const ice::sonic::TF_StringOps& token) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_basic_auth(
        const ice::sonic::TF_StringOps& username,
        const ice::sonic::TF_StringOps& password
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_authorization(const ice::sonic::TF_StringOps& out_authorization) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_addr(const ice::sonic::TF_StringOps& addr) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_no_decompress(int enabled) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_timeout(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_timeout(int64_t* out_timeout_ms) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_stream_id(uint32_t stream_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_stream_id(uint32_t* out_stream_id) noexcept = 0;

    static TF_RequestOps* get_generic_vtable()
    {
        static TF_RequestOps vtable = {
            .struct_size = TF_REQUEST_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_RequestOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_RequestOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .set_method =
                [](TF_Request* request, const TF_String* method, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_method(ice::sonic::TF_StringOps::wrap(method));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_method =
                [](TF_Request* request, TF_String* out_method) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_method(ice::sonic::TF_StringOps::wrap(out_method));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_path =
                [](TF_Request* request, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_path(ice::sonic::TF_StringOps::wrap(path));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_path =
                [](TF_Request* request, TF_String* out_path) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_path(ice::sonic::TF_StringOps::wrap(out_path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_scheme =
                [](TF_Request* request, const TF_String* scheme, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_scheme(ice::sonic::TF_StringOps::wrap(scheme));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_scheme =
                [](TF_Request* request, TF_String* out_scheme) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_scheme(ice::sonic::TF_StringOps::wrap(out_scheme));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_authority =
                [](TF_Request* request, const TF_String* authority, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_authority(ice::sonic::TF_StringOps::wrap(authority));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_authority =
                [](TF_Request* request, TF_String* out_authority) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_authority(ice::sonic::TF_StringOps::wrap(out_authority));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_header =
                [](TF_Request* request,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_header(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_StringOps::wrap(value)
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
                auto* self = TF_RequestOps::create(request);
                auto res = self->add_header(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_StringOps::wrap(value)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .remove_header =
                [](TF_Request* request, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->remove_header(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .find_header =
                [](TF_Request* request, const TF_String* name, const TF_String** out_value) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->find_header(ice::sonic::TF_StringOps::wrap(name), out_value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .clear_headers =
                [](TF_Request* request, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->clear_headers();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_headers =
                [](TF_Request* request, TF_Map* out_headers, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_headers(ice::sonic::TF_MapOps::wrap(out_headers));
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
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_query_param(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_StringOps::wrap(value)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_query_params =
                [](TF_Request* request, TF_Map* out_params, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_query_params(ice::sonic::TF_MapOps::wrap(out_params));
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
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_body(data, length);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_body =
                [](TF_Request* request, const void** out_data, size_t* out_length) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_body(out_data, out_length);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_content_type =
                [](TF_Request* request,
                   const TF_String* content_type,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_content_type(ice::sonic::TF_StringOps::wrap(content_type));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_content_type =
                [](TF_Request* request, TF_String* out_content_type) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_content_type(ice::sonic::TF_StringOps::wrap(out_content_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_accept =
                [](TF_Request* request, const TF_String* accept, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_accept(ice::sonic::TF_StringOps::wrap(accept));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_accept =
                [](TF_Request* request, TF_String* out_accept) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_accept(ice::sonic::TF_StringOps::wrap(out_accept));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_user_agent =
                [](TF_Request* request, const TF_String* user_agent, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_user_agent(ice::sonic::TF_StringOps::wrap(user_agent));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_user_agent =
                [](TF_Request* request, TF_String* out_user_agent) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_user_agent(ice::sonic::TF_StringOps::wrap(out_user_agent));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_bearer_auth =
                [](TF_Request* request, const TF_String* token, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_bearer_auth(ice::sonic::TF_StringOps::wrap(token));
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
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_basic_auth(
                    ice::sonic::TF_StringOps::wrap(username),
                    ice::sonic::TF_StringOps::wrap(password)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_authorization =
                [](TF_Request* request, TF_String* out_authorization) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res =
                    self->get_authorization(ice::sonic::TF_StringOps::wrap(out_authorization));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_addr =
                [](TF_Request* request, const TF_String* addr, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_addr(ice::sonic::TF_StringOps::wrap(addr));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_no_decompress =
                [](TF_Request* request, int enabled, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_no_decompress(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_timeout =
                [](TF_Request* request, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_timeout(timeout_ms);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_timeout =
                [](TF_Request* request, int64_t* out_timeout_ms) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_timeout(out_timeout_ms);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_stream_id =
                [](TF_Request* request, uint32_t stream_id, TF_Status* out_status) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->set_stream_id(stream_id);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stream_id =
                [](TF_Request* request, uint32_t* out_stream_id) noexcept
            {
                auto* self = TF_RequestOps::create(request);
                auto res = self->get_stream_id(out_stream_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
