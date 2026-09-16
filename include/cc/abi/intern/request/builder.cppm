// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/request/request.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/request/request.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_request;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Request
{
public:
    static Request* create(void* ctx) noexcept
    {
        return static_cast<Request*>(ctx);
    }

    template<typename HandleT>
    static Request* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Request*>(handle);
    }

    virtual ~Request() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_request(uint32_t stream_id) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> destroy_request() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_method(const ice::sonic::String& method) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_method(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_path(const ice::sonic::String& path) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_path(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_scheme(const ice::sonic::String& scheme) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_scheme(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_authority(const ice::sonic::String& authority) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_authority(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_header(const ice::sonic::String& name, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    add_header(const ice::sonic::String& name, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    remove_header(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    find_header(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> clear_headers() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_headers(const ice::sonic::Map& out_headers) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_query_param(const ice::sonic::String& name, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_query_params(const ice::sonic::Map& out_params) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_body(const void* data, size_t length) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_body(const void** out_data, size_t* out_length) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_content_type(const ice::sonic::String& content_type) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_content_type(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_accept(const ice::sonic::String& accept) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_accept(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_user_agent(const ice::sonic::String& user_agent) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_user_agent(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_bearer_auth(const ice::sonic::String& token) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_basic_auth(
        const ice::sonic::String& username,
        const ice::sonic::String& password
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_authorization(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_addr(const ice::sonic::String& addr) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_no_decompress(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_timeout(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_timeout() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_stream_id(uint32_t stream_id) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_stream_id() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Request* get_generic_vtable()
    {
        static TF_Request vtable = {
            .struct_size = TF_REQUEST_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Request::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Request::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .new_request =
                [](void* plugin_context, uint32_t stream_id, TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(plugin_context);
                auto res = self->new_request(stream_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .destroy_request =
                [](TF_Request_Handle* request) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->destroy_request();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_method =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* method,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_method(ice::sonic::String::wrap(method));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_method =
                [](TF_Request_Handle* request, TF_String* out) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_method(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_path =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* path,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_path(ice::sonic::String::wrap(path));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_path =
                [](TF_Request_Handle* request, TF_String* out) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_path(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_scheme =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* scheme,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_scheme(ice::sonic::String::wrap(scheme));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_scheme =
                [](TF_Request_Handle* request, TF_String* out) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_scheme(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_authority =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* authority,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_authority(ice::sonic::String::wrap(authority));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_authority =
                [](TF_Request_Handle* request, TF_String* out) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_authority(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_header =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* name,
                   const TF_String_Handle* value,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_header(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .add_header =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* name,
                   const TF_String_Handle* value,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->add_header(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .remove_header =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* name,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->remove_header(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .find_header =
                [](TF_Request_Handle* request, const TF_String_Handle* name) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->find_header(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .clear_headers =
                [](TF_Request_Handle* request, TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->clear_headers();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_headers =
                [](TF_Request_Handle* request,
                   TF_Map_Handle* out_headers,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_headers(ice::sonic::Map::wrap(out_headers));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_query_param =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* name,
                   const TF_String_Handle* value,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_query_param(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_query_params =
                [](TF_Request_Handle* request,
                   TF_Map_Handle* out_params,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_query_params(ice::sonic::Map::wrap(out_params));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_body =
                [](TF_Request_Handle* request,
                   const void* data,
                   size_t length,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_body(data, length);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_body =
                [](TF_Request_Handle* request, const void** out_data, size_t* out_length) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_body(out_data, out_length);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_content_type =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* content_type,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_content_type(ice::sonic::String::wrap(content_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_content_type =
                [](TF_Request_Handle* request, TF_String* out) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_content_type(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_accept =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* accept,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_accept(ice::sonic::String::wrap(accept));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_accept =
                [](TF_Request_Handle* request, TF_String* out) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_accept(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_user_agent =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* user_agent,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_user_agent(ice::sonic::String::wrap(user_agent));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_user_agent =
                [](TF_Request_Handle* request, TF_String* out) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_user_agent(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_bearer_auth =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* token,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_bearer_auth(ice::sonic::String::wrap(token));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_basic_auth =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* username,
                   const TF_String_Handle* password,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_basic_auth(
                    ice::sonic::String::wrap(username),
                    ice::sonic::String::wrap(password)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_authorization =
                [](TF_Request_Handle* request, TF_String* out) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_authorization(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_addr =
                [](TF_Request_Handle* request,
                   const TF_String_Handle* addr,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_addr(ice::sonic::String::wrap(addr));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_no_decompress =
                [](TF_Request_Handle* request, int enabled, TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_no_decompress(enabled);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_timeout =
                [](TF_Request_Handle* request,
                   int64_t timeout_ms,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_timeout(timeout_ms);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_timeout =
                [](TF_Request_Handle* request) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_timeout();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_stream_id =
                [](TF_Request_Handle* request,
                   uint32_t stream_id,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->set_stream_id(stream_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_stream_id =
                [](TF_Request_Handle* request) noexcept
            {
                auto* self = Request::create(request);
                auto res = self->get_stream_id();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
