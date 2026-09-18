// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/response.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/response.h"

export module cc_ice_builder_io:response;

import std;

export namespace ice::builder {

class TF_ResponseOps
{
public:
    static TF_ResponseOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ResponseOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ResponseOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ResponseOps*>(handle->plugin_data);
    }

    virtual ~TF_ResponseOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_status(int32_t status_code) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_status(int32_t* out_status_code) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_status_text(const ice::sonic::TF_StringOps& out_status_text) noexcept = 0;
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
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_headers(const ice::sonic::TF_MapOps& out_headers) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_body(const void* data, size_t length) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_body(const void** out_data, size_t* out_length) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_keep_alive(int enabled) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_keep_alive(int* out_keep_alive) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_cookie(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_StringOps& value,
        const ice::sonic::TF_MapOps& attributes
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_set_cookies(const ice::sonic::TF_VectorOps& out_cookies) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_content_type(const ice::sonic::TF_StringOps& out_content_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_content_length(int64_t* out_length) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_location(const ice::sonic::TF_StringOps& out_location) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_etag(const ice::sonic::TF_StringOps& out_etag) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_date(const ice::sonic::TF_StringOps& out_date) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_server(const ice::sonic::TF_StringOps& out_server) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_cache_control(const ice::sonic::TF_StringOps& out_cache_control) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_last_modified(const ice::sonic::TF_StringOps& out_last_modified) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_informational(int* out_result) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> is_success(int* out_result) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_redirection(int* out_result) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_client_error(int* out_result) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_server_error(int* out_result) noexcept = 0;

    static TF_ResponseOps* get_generic_vtable()
    {
        static TF_ResponseOps vtable = {
            .struct_size = TF_RESPONSE_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_ResponseOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_ResponseOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .set_status =
                [](TF_Response* response, int32_t status_code, TF_Status* out_status) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->set_status(status_code);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_status =
                [](TF_Response* response, int32_t* out_status_code) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->get_status(out_status_code);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_status_text =
                [](TF_Response* response, TF_String* out_status_text) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->get_status_text(ice::sonic::TF_StringOps::wrap(out_status_text));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_header =
                [](TF_Response* response,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->set_header(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_StringOps::wrap(value)
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
                auto* self = TF_ResponseOps::create(response);
                auto res = self->add_header(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_StringOps::wrap(value)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .remove_header =
                [](TF_Response* response, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->remove_header(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .find_header =
                [](TF_Response* response,
                   const TF_String* name,
                   const TF_String** out_value) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->find_header(ice::sonic::TF_StringOps::wrap(name), out_value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_headers =
                [](TF_Response* response, TF_Map* out_headers, TF_Status* out_status) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->get_headers(ice::sonic::TF_MapOps::wrap(out_headers));
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
                auto* self = TF_ResponseOps::create(response);
                auto res = self->set_body(data, length);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_body =
                [](TF_Response* response, const void** out_data, size_t* out_length) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->get_body(out_data, out_length);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_keep_alive =
                [](TF_Response* response, int enabled, TF_Status* out_status) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->set_keep_alive(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .is_keep_alive =
                [](TF_Response* response, int* out_keep_alive) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->is_keep_alive(out_keep_alive);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_cookie =
                [](TF_Response* response,
                   const TF_String* name,
                   const TF_String* value,
                   const TF_Map* attributes,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->set_cookie(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_StringOps::wrap(value),
                    ice::sonic::TF_MapOps::wrap(attributes)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_set_cookies =
                [](TF_Response* response, TF_Vector* out_cookies, TF_Status* out_status) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->get_set_cookies(ice::sonic::TF_VectorOps::wrap(out_cookies));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_content_type =
                [](TF_Response* response, TF_String* out_content_type) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->get_content_type(ice::sonic::TF_StringOps::wrap(out_content_type));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_content_length =
                [](TF_Response* response, int64_t* out_length) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->get_content_length(out_length);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_location =
                [](TF_Response* response, TF_String* out_location) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->get_location(ice::sonic::TF_StringOps::wrap(out_location));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_etag =
                [](TF_Response* response, TF_String* out_etag) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->get_etag(ice::sonic::TF_StringOps::wrap(out_etag));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_date =
                [](TF_Response* response, TF_String* out_date) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->get_date(ice::sonic::TF_StringOps::wrap(out_date));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_server =
                [](TF_Response* response, TF_String* out_server) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->get_server(ice::sonic::TF_StringOps::wrap(out_server));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_cache_control =
                [](TF_Response* response, TF_String* out_cache_control) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res =
                    self->get_cache_control(ice::sonic::TF_StringOps::wrap(out_cache_control));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_last_modified =
                [](TF_Response* response, TF_String* out_last_modified) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res =
                    self->get_last_modified(ice::sonic::TF_StringOps::wrap(out_last_modified));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_informational =
                [](TF_Response* response, int* out_result) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->is_informational(out_result);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_success =
                [](TF_Response* response, int* out_result) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->is_success(out_result);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_redirection =
                [](TF_Response* response, int* out_result) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->is_redirection(out_result);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_client_error =
                [](TF_Response* response, int* out_result) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->is_client_error(out_result);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_server_error =
                [](TF_Response* response, int* out_result) noexcept
            {
                auto* self = TF_ResponseOps::create(response);
                auto res = self->is_server_error(out_result);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
