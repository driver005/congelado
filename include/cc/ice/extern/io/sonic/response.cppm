// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/response.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/response.h"

export module cc_ice_extern_io_sonic:response;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_ResponseOps : public ice::sonic::Runtime<::TF_ResponseOps, ::TF_Response>
{
public:
    template<typename Registry>
    TF_ResponseOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_ResponseOps(
        Registry& registry,
        ::TF_Response* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_ResponseOps(const ::TF_ResponseOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ResponseOps(const ::TF_ResponseOps* ops, ::TF_Response* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void set_status(int32_t status_code, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_status(get_handle(), status_code, out_status.get_handle());
    }

    void get_status(int32_t* out_status_code) const noexcept
    {
        m_ops->get_status(get_handle(), out_status_code);
    }

    void get_status_text(const ice::sonic::String& out_status_text) const noexcept
    {
        m_ops->get_status_text(get_handle(), out_status_text.get_handle());
    }

    void set_header(
        const ice::sonic::String& name,
        const ice::sonic::String& value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_header(
            get_handle(),
            name.get_handle(),
            value.get_handle(),
            out_status.get_handle()
        );
    }

    void add_header(
        const ice::sonic::String& name,
        const ice::sonic::String& value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_header(
            get_handle(),
            name.get_handle(),
            value.get_handle(),
            out_status.get_handle()
        );
    }

    void remove_header(
        const ice::sonic::String& name,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->remove_header(get_handle(), name.get_handle(), out_status.get_handle());
    }

    void find_header(const ice::sonic::String& name, const TF_String** out_value) const noexcept
    {
        m_ops->find_header(get_handle(), name.get_handle(), out_value);
    }

    void get_headers(
        const ice::sonic::TF_MapOps& out_headers,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_headers(get_handle(), out_headers.get_handle(), out_status.get_handle());
    }

    void set_body(
        const void* data,
        size_t length,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_body(get_handle(), data, length, out_status.get_handle());
    }

    void get_body(const void** out_data, size_t* out_length) const noexcept
    {
        m_ops->get_body(get_handle(), out_data, out_length);
    }

    void set_keep_alive(int enabled, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_keep_alive(get_handle(), enabled, out_status.get_handle());
    }

    void is_keep_alive(int* out_keep_alive) const noexcept
    {
        m_ops->is_keep_alive(get_handle(), out_keep_alive);
    }

    void set_cookie(
        const ice::sonic::String& name,
        const ice::sonic::String& value,
        const ice::sonic::TF_MapOps& attributes,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_cookie(
            get_handle(),
            name.get_handle(),
            value.get_handle(),
            attributes.get_handle(),
            out_status.get_handle()
        );
    }

    void get_set_cookies(
        const ice::sonic::TF_VectorOps& out_cookies,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_set_cookies(get_handle(), out_cookies.get_handle(), out_status.get_handle());
    }

    void get_content_type(const ice::sonic::String& out_content_type) const noexcept
    {
        m_ops->get_content_type(get_handle(), out_content_type.get_handle());
    }

    void get_content_length(int64_t* out_length) const noexcept
    {
        m_ops->get_content_length(get_handle(), out_length);
    }

    void get_location(const ice::sonic::String& out_location) const noexcept
    {
        m_ops->get_location(get_handle(), out_location.get_handle());
    }

    void get_etag(const ice::sonic::String& out_etag) const noexcept
    {
        m_ops->get_etag(get_handle(), out_etag.get_handle());
    }

    void get_date(const ice::sonic::String& out_date) const noexcept
    {
        m_ops->get_date(get_handle(), out_date.get_handle());
    }

    void get_server(const ice::sonic::String& out_server) const noexcept
    {
        m_ops->get_server(get_handle(), out_server.get_handle());
    }

    void get_cache_control(const ice::sonic::String& out_cache_control) const noexcept
    {
        m_ops->get_cache_control(get_handle(), out_cache_control.get_handle());
    }

    void get_last_modified(const ice::sonic::String& out_last_modified) const noexcept
    {
        m_ops->get_last_modified(get_handle(), out_last_modified.get_handle());
    }

    void is_informational(int* out_result) const noexcept
    {
        m_ops->is_informational(get_handle(), out_result);
    }

    void is_success(int* out_result) const noexcept
    {
        m_ops->is_success(get_handle(), out_result);
    }

    void is_redirection(int* out_result) const noexcept
    {
        m_ops->is_redirection(get_handle(), out_result);
    }

    void is_client_error(int* out_result) const noexcept
    {
        m_ops->is_client_error(get_handle(), out_result);
    }

    void is_server_error(int* out_result) const noexcept
    {
        m_ops->is_server_error(get_handle(), out_result);
    }
};

} // namespace ice::sonic
