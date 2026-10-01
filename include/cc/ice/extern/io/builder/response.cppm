// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/response.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/response.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_io_builder:response;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ResponseOps
{
public:
    explicit TF_ResponseOps(
        const ::TF_MapOps* TF_MapOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_VectorOps* TF_VectorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_MapOps_ops = TF_MapOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_TF_VectorOps_ops = TF_VectorOps_ops;
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
    virtual void set_status(int32_t status_code, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_status(int32_t* out_status_code) noexcept = 0;
    virtual void get_status_text(const ice::sonic::String& out_status_text) noexcept = 0;
    virtual void set_header(
        const ice::sonic::String& name,
        const ice::sonic::String& value,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void add_header(
        const ice::sonic::String& name,
        const ice::sonic::String& value,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void remove_header(
        const ice::sonic::String& name,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    find_header(const ice::sonic::String& name, const TF_String** out_value) noexcept = 0;
    virtual void get_headers(
        const ice::sonic::TF_MapOps& out_headers,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    set_body(const void* data, size_t length, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_body(const void** out_data, size_t* out_length) noexcept = 0;
    virtual void set_keep_alive(int enabled, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void is_keep_alive(int* out_keep_alive) noexcept = 0;
    virtual void set_cookie(
        const ice::sonic::String& name,
        const ice::sonic::String& value,
        const ice::sonic::TF_MapOps& attributes,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_set_cookies(
        const ice::sonic::TF_VectorOps& out_cookies,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
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

    void get_generic_vtable(void (*create)(::TF_Response*)) noexcept
    {
        m_vtable = ::TF_ResponseOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ResponseOps, is_server_error),

            .create = create,
            .destroy =
                [](TF_Response* handle) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Response* response, TF_String* out_name) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .set_status =
                [](TF_Response* response, int32_t status_code, TF_Status* out_status) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.set_status(
                    status_code,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_status =
                [](TF_Response* response, int32_t* out_status_code) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_status(out_status_code);
            },
            .get_status_text =
                [](TF_Response* response, TF_String* out_status_text) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_status_text(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_status_text)
                );
            },
            .set_header =
                [](TF_Response* response,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.set_header(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::String>{}, value),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .add_header =
                [](TF_Response* response,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.add_header(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::String>{}, value),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .remove_header =
                [](TF_Response* response, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.remove_header(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .find_header =
                [](TF_Response* response,
                   const TF_String* name,
                   const TF_String** out_value) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.find_header(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    out_value
                );
            },
            .get_headers =
                [](TF_Response* response, TF_Map* out_headers, TF_Status* out_status) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_headers(
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, out_headers),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_body =
                [](TF_Response* response,
                   const void* data,
                   size_t length,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.set_body(
                    data,
                    length,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_body =
                [](TF_Response* response, const void** out_data, size_t* out_length) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_body(out_data, out_length);
            },
            .set_keep_alive =
                [](TF_Response* response, int enabled, TF_Status* out_status) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.set_keep_alive(
                    enabled,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .is_keep_alive =
                [](TF_Response* response, int* out_keep_alive) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.is_keep_alive(out_keep_alive);
            },
            .set_cookie =
                [](TF_Response* response,
                   const TF_String* name,
                   const TF_String* value,
                   const TF_Map* attributes,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.set_cookie(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::String>{}, value),
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, attributes),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_set_cookies =
                [](TF_Response* response, TF_Vector* out_cookies, TF_Status* out_status) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_set_cookies(
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, out_cookies),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_content_type =
                [](TF_Response* response, TF_String* out_content_type) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_content_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_content_type)
                );
            },
            .get_content_length =
                [](TF_Response* response, int64_t* out_length) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_content_length(out_length);
            },
            .get_location =
                [](TF_Response* response, TF_String* out_location) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_location(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_location)
                );
            },
            .get_etag =
                [](TF_Response* response, TF_String* out_etag) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_etag(self.wrap(std::type_identity<ice::sonic::String>{}, out_etag));
            },
            .get_date =
                [](TF_Response* response, TF_String* out_date) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_date(self.wrap(std::type_identity<ice::sonic::String>{}, out_date));
            },
            .get_server =
                [](TF_Response* response, TF_String* out_server) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_server(self.wrap(std::type_identity<ice::sonic::String>{}, out_server));
            },
            .get_cache_control =
                [](TF_Response* response, TF_String* out_cache_control) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_cache_control(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_cache_control)
                );
            },
            .get_last_modified =
                [](TF_Response* response, TF_String* out_last_modified) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.get_last_modified(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_last_modified)
                );
            },
            .is_informational =
                [](TF_Response* response, int* out_result) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.is_informational(out_result);
            },
            .is_success =
                [](TF_Response* response, int* out_result) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.is_success(out_result);
            },
            .is_redirection =
                [](TF_Response* response, int* out_result) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.is_redirection(out_result);
            },
            .is_client_error =
                [](TF_Response* response, int* out_result) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.is_client_error(out_result);
            },
            .is_server_error =
                [](TF_Response* response, int* out_result) noexcept
            {
                auto& self = TF_ResponseOps::from_handle(response);
                self.is_server_error(out_result);
            },

        };
    }

    ice::sonic::TF_MapOps
    wrap(std::type_identity<ice::sonic::TF_MapOps>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::TF_MapOps{m_TF_MapOps_ops, const_cast<::TF_Map*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    ice::sonic::TF_VectorOps
    wrap(std::type_identity<ice::sonic::TF_VectorOps>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::TF_VectorOps{m_TF_VectorOps_ops, const_cast<::TF_Vector*>(handle)};
    }

    const ::TF_ResponseOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Response& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_ResponseOps*>(&m_vtable));
    }

private:
    ::TF_ResponseOps m_vtable;
    ::TF_Response m_handle;

    const ::TF_MapOps* m_TF_MapOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_VectorOps* m_TF_VectorOps_ops{nullptr};
};

} // namespace ice::builder
