// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/request.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/request.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_io_sonic:request;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_RequestOps : public ice::sonic::Runtime<::TF_RequestOps, ::TF_Request>
{
public:
    TF_RequestOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_RequestOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Request* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_RequestOps(const ::TF_RequestOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_RequestOps(const ::TF_RequestOps* ops, ::TF_Request* handle) noexcept :
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

    void set_method(
        const ice::sonic::String& method,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_method(get_handle(), method.get_handle(), out_status.get_handle());
    }

    void get_method(const ice::sonic::String& out_method) const noexcept
    {
        m_ops->get_method(get_handle(), out_method.get_handle());
    }

    void set_path(
        const ice::sonic::String& path,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_path(get_handle(), path.get_handle(), out_status.get_handle());
    }

    void get_path(const ice::sonic::String& out_path) const noexcept
    {
        m_ops->get_path(get_handle(), out_path.get_handle());
    }

    void set_scheme(
        const ice::sonic::String& scheme,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_scheme(get_handle(), scheme.get_handle(), out_status.get_handle());
    }

    void get_scheme(const ice::sonic::String& out_scheme) const noexcept
    {
        m_ops->get_scheme(get_handle(), out_scheme.get_handle());
    }

    void set_authority(
        const ice::sonic::String& authority,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_authority(get_handle(), authority.get_handle(), out_status.get_handle());
    }

    void get_authority(const ice::sonic::String& out_authority) const noexcept
    {
        m_ops->get_authority(get_handle(), out_authority.get_handle());
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

    void clear_headers(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->clear_headers(get_handle(), out_status.get_handle());
    }

    void get_headers(
        const ice::sonic::TF_MapOps& out_headers,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_headers(get_handle(), out_headers.get_handle(), out_status.get_handle());
    }

    void set_query_param(
        const ice::sonic::String& name,
        const ice::sonic::String& value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_query_param(
            get_handle(),
            name.get_handle(),
            value.get_handle(),
            out_status.get_handle()
        );
    }

    void get_query_params(
        const ice::sonic::TF_MapOps& out_params,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_query_params(get_handle(), out_params.get_handle(), out_status.get_handle());
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

    void set_content_type(
        const ice::sonic::String& content_type,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_content_type(get_handle(), content_type.get_handle(), out_status.get_handle());
    }

    void get_content_type(const ice::sonic::String& out_content_type) const noexcept
    {
        m_ops->get_content_type(get_handle(), out_content_type.get_handle());
    }

    void set_accept(
        const ice::sonic::String& accept,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_accept(get_handle(), accept.get_handle(), out_status.get_handle());
    }

    void get_accept(const ice::sonic::String& out_accept) const noexcept
    {
        m_ops->get_accept(get_handle(), out_accept.get_handle());
    }

    void set_user_agent(
        const ice::sonic::String& user_agent,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_user_agent(get_handle(), user_agent.get_handle(), out_status.get_handle());
    }

    void get_user_agent(const ice::sonic::String& out_user_agent) const noexcept
    {
        m_ops->get_user_agent(get_handle(), out_user_agent.get_handle());
    }

    void set_bearer_auth(
        const ice::sonic::String& token,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_bearer_auth(get_handle(), token.get_handle(), out_status.get_handle());
    }

    void set_basic_auth(
        const ice::sonic::String& username,
        const ice::sonic::String& password,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_basic_auth(
            get_handle(),
            username.get_handle(),
            password.get_handle(),
            out_status.get_handle()
        );
    }

    void get_authorization(const ice::sonic::String& out_authorization) const noexcept
    {
        m_ops->get_authorization(get_handle(), out_authorization.get_handle());
    }

    void set_addr(
        const ice::sonic::String& addr,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_addr(get_handle(), addr.get_handle(), out_status.get_handle());
    }

    void set_no_decompress(int enabled, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_no_decompress(get_handle(), enabled, out_status.get_handle());
    }

    void set_timeout(int64_t timeout_ms, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_timeout(get_handle(), timeout_ms, out_status.get_handle());
    }

    void get_timeout(int64_t* out_timeout_ms) const noexcept
    {
        m_ops->get_timeout(get_handle(), out_timeout_ms);
    }

    void set_stream_id(uint32_t stream_id, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_stream_id(get_handle(), stream_id, out_status.get_handle());
    }

    void get_stream_id(uint32_t* out_stream_id) const noexcept
    {
        m_ops->get_stream_id(get_handle(), out_stream_id);
    }
};

} // namespace ice::sonic
