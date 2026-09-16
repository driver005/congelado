// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/response/response.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/response/response.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_response;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Response
{
public:
    static Response* create(void* ctx) noexcept
    {
        return static_cast<Response*>(ctx);
    }

    template<typename HandleT>
    static Response* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Response*>(handle);
    }

    virtual ~Response() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_response(uint32_t stream_id) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> destroy_response() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_status(int32_t status_code) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_status() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_status_text(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_header(const ice::sonic::String& name, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    add_header(const ice::sonic::String& name, const ice::sonic::String& value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    remove_header(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    find_header(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_headers(const ice::sonic::Map& out_headers) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_body(const void* data, size_t length) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_body(const void** out_data, size_t* out_length) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_keep_alive(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_keep_alive() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_cookie(
        const ice::sonic::String& name,
        const ice::sonic::String& value,
        const ice::sonic::Map& attributes
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_set_cookies(const ice::sonic::Vector& out_cookies) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_content_type(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_content_length() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_location(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_etag(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_date(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_server(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_cache_control(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_last_modified(TF_String* out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_informational() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_success() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_redirection() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_client_error() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_server_error() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Response* get_generic_vtable()
    {
        static TF_Response vtable = {
            .struct_size = TF_RESPONSE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Response::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Response::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .new_response =
                [](void* plugin_context, uint32_t stream_id, TF_Status_Handle* status) noexcept
            {
                auto* self = Response::create(plugin_context);
                auto res = self->new_response(stream_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .destroy_response =
                [](TF_Response_Handle* response) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->destroy_response();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_status =
                [](TF_Response_Handle* response,
                   int32_t status_code,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->set_status(status_code);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_status =
                [](TF_Response_Handle* response) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_status();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_status_text =
                [](TF_Response_Handle* response, TF_String* out) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_status_text(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_header =
                [](TF_Response_Handle* response,
                   const TF_String_Handle* name,
                   const TF_String_Handle* value,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->set_header(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .add_header =
                [](TF_Response_Handle* response,
                   const TF_String_Handle* name,
                   const TF_String_Handle* value,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->add_header(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .remove_header =
                [](TF_Response_Handle* response,
                   const TF_String_Handle* name,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->remove_header(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .find_header =
                [](TF_Response_Handle* response, const TF_String_Handle* name) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->find_header(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_headers =
                [](TF_Response_Handle* response,
                   TF_Map_Handle* out_headers,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_headers(ice::sonic::Map::wrap(out_headers));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_body =
                [](TF_Response_Handle* response,
                   const void* data,
                   size_t length,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->set_body(data, length);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_body =
                [](TF_Response_Handle* response, const void** out_data, size_t* out_length) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_body(out_data, out_length);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_keep_alive =
                [](TF_Response_Handle* response, int enabled, TF_Status_Handle* status) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->set_keep_alive(enabled);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_keep_alive =
                [](TF_Response_Handle* response) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->is_keep_alive();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_cookie =
                [](TF_Response_Handle* response,
                   const TF_String_Handle* name,
                   const TF_String_Handle* value,
                   const TF_Map_Handle* attributes,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->set_cookie(
                    ice::sonic::String::wrap(name),
                    ice::sonic::String::wrap(value),
                    ice::sonic::Map::wrap(attributes)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_set_cookies =
                [](TF_Response_Handle* response,
                   TF_Vector_Handle* out_cookies,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_set_cookies(ice::sonic::Vector::wrap(out_cookies));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_content_type =
                [](TF_Response_Handle* response, TF_String* out) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_content_type(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_content_length =
                [](TF_Response_Handle* response) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_content_length();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_location =
                [](TF_Response_Handle* response, TF_String* out) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_location(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_etag =
                [](TF_Response_Handle* response, TF_String* out) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_etag(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_date =
                [](TF_Response_Handle* response, TF_String* out) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_date(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_server =
                [](TF_Response_Handle* response, TF_String* out) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_server(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_cache_control =
                [](TF_Response_Handle* response, TF_String* out) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_cache_control(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_last_modified =
                [](TF_Response_Handle* response, TF_String* out) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->get_last_modified(out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_informational =
                [](TF_Response_Handle* response) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->is_informational();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_success =
                [](TF_Response_Handle* response) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->is_success();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_redirection =
                [](TF_Response_Handle* response) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->is_redirection();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_client_error =
                [](TF_Response_Handle* response) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->is_client_error();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_server_error =
                [](TF_Response_Handle* response) noexcept
            {
                auto* self = Response::create(response);
                auto res = self->is_server_error();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
