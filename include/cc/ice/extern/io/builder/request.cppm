// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/request.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/request.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_io_builder:request;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_RequestOps
{
public:
    explicit TF_RequestOps(
        const ::TF_MapOps* TF_MapOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_MapOps_ops = TF_MapOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void
    set_method(const ice::sonic::String& method, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_method(const ice::sonic::String& out_method) noexcept = 0;
    virtual void
    set_path(const ice::sonic::String& path, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_path(const ice::sonic::String& out_path) noexcept = 0;
    virtual void
    set_scheme(const ice::sonic::String& scheme, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_scheme(const ice::sonic::String& out_scheme) noexcept = 0;
    virtual void set_authority(
        const ice::sonic::String& authority,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_authority(const ice::sonic::String& out_authority) noexcept = 0;
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
    virtual void clear_headers(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_headers(
        const ice::sonic::TF_MapOps& out_headers,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_query_param(
        const ice::sonic::String& name,
        const ice::sonic::String& value,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_query_params(
        const ice::sonic::TF_MapOps& out_params,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    set_body(const void* data, size_t length, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_body(const void** out_data, size_t* out_length) noexcept = 0;
    virtual void set_content_type(
        const ice::sonic::String& content_type,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_content_type(const ice::sonic::String& out_content_type) noexcept = 0;
    virtual void
    set_accept(const ice::sonic::String& accept, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_accept(const ice::sonic::String& out_accept) noexcept = 0;
    virtual void set_user_agent(
        const ice::sonic::String& user_agent,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_user_agent(const ice::sonic::String& out_user_agent) noexcept = 0;
    virtual void set_bearer_auth(
        const ice::sonic::String& token,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_basic_auth(
        const ice::sonic::String& username,
        const ice::sonic::String& password,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_authorization(const ice::sonic::String& out_authorization) noexcept = 0;
    virtual void
    set_addr(const ice::sonic::String& addr, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void set_no_decompress(int enabled, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void set_timeout(int64_t timeout_ms, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_timeout(int64_t* out_timeout_ms) noexcept = 0;
    virtual void
    set_stream_id(uint32_t stream_id, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_stream_id(uint32_t* out_stream_id) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Request*)) noexcept
    {
        m_vtable = ::TF_RequestOps{
            .struct_size = TF_OFFSET_OF_END(::TF_RequestOps, get_stream_id),

            .create = create,
            .destroy =
                [](TF_Request* handle) noexcept
            {
                auto& self = TF_RequestOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Request* request, TF_String* out_name) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .set_method =
                [](TF_Request* request, const TF_String* method, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_method(
                    self.wrap(std::type_identity<ice::sonic::String>{}, method),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_method =
                [](TF_Request* request, TF_String* out_method) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_method(self.wrap(std::type_identity<ice::sonic::String>{}, out_method));
            },
            .set_path =
                [](TF_Request* request, const TF_String* path, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_path(
                    self.wrap(std::type_identity<ice::sonic::String>{}, path),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_path =
                [](TF_Request* request, TF_String* out_path) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_path(self.wrap(std::type_identity<ice::sonic::String>{}, out_path));
            },
            .set_scheme =
                [](TF_Request* request, const TF_String* scheme, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_scheme(
                    self.wrap(std::type_identity<ice::sonic::String>{}, scheme),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_scheme =
                [](TF_Request* request, TF_String* out_scheme) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_scheme(self.wrap(std::type_identity<ice::sonic::String>{}, out_scheme));
            },
            .set_authority =
                [](TF_Request* request, const TF_String* authority, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_authority(
                    self.wrap(std::type_identity<ice::sonic::String>{}, authority),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_authority =
                [](TF_Request* request, TF_String* out_authority) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_authority(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_authority)
                );
            },
            .set_header =
                [](TF_Request* request,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_header(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::String>{}, value),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .add_header =
                [](TF_Request* request,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.add_header(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::String>{}, value),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .remove_header =
                [](TF_Request* request, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.remove_header(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .find_header =
                [](TF_Request* request, const TF_String* name, const TF_String** out_value) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.find_header(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    out_value
                );
            },
            .clear_headers =
                [](TF_Request* request, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.clear_headers(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .get_headers =
                [](TF_Request* request, TF_Map* out_headers, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_headers(
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, out_headers),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_query_param =
                [](TF_Request* request,
                   const TF_String* name,
                   const TF_String* value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_query_param(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::String>{}, value),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_query_params =
                [](TF_Request* request, TF_Map* out_params, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_query_params(
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, out_params),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_body =
                [](TF_Request* request,
                   const void* data,
                   size_t length,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_body(
                    data,
                    length,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_body =
                [](TF_Request* request, const void** out_data, size_t* out_length) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_body(out_data, out_length);
            },
            .set_content_type =
                [](TF_Request* request,
                   const TF_String* content_type,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_content_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, content_type),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_content_type =
                [](TF_Request* request, TF_String* out_content_type) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_content_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_content_type)
                );
            },
            .set_accept =
                [](TF_Request* request, const TF_String* accept, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_accept(
                    self.wrap(std::type_identity<ice::sonic::String>{}, accept),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_accept =
                [](TF_Request* request, TF_String* out_accept) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_accept(self.wrap(std::type_identity<ice::sonic::String>{}, out_accept));
            },
            .set_user_agent =
                [](TF_Request* request, const TF_String* user_agent, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_user_agent(
                    self.wrap(std::type_identity<ice::sonic::String>{}, user_agent),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_user_agent =
                [](TF_Request* request, TF_String* out_user_agent) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_user_agent(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_user_agent)
                );
            },
            .set_bearer_auth =
                [](TF_Request* request, const TF_String* token, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_bearer_auth(
                    self.wrap(std::type_identity<ice::sonic::String>{}, token),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_basic_auth =
                [](TF_Request* request,
                   const TF_String* username,
                   const TF_String* password,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_basic_auth(
                    self.wrap(std::type_identity<ice::sonic::String>{}, username),
                    self.wrap(std::type_identity<ice::sonic::String>{}, password),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_authorization =
                [](TF_Request* request, TF_String* out_authorization) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_authorization(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_authorization)
                );
            },
            .set_addr =
                [](TF_Request* request, const TF_String* addr, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_addr(
                    self.wrap(std::type_identity<ice::sonic::String>{}, addr),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_no_decompress =
                [](TF_Request* request, int enabled, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_no_decompress(
                    enabled,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_timeout =
                [](TF_Request* request, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_timeout(
                    timeout_ms,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_timeout =
                [](TF_Request* request, int64_t* out_timeout_ms) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_timeout(out_timeout_ms);
            },
            .set_stream_id =
                [](TF_Request* request, uint32_t stream_id, TF_Status* out_status) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.set_stream_id(
                    stream_id,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stream_id =
                [](TF_Request* request, uint32_t* out_stream_id) noexcept
            {
                auto& self = TF_RequestOps::from_handle(request);
                self.get_stream_id(out_stream_id);
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

    const ::TF_RequestOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Request& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_RequestOps*>(&m_vtable));
    }

private:
    ::TF_RequestOps m_vtable;
    ::TF_Request m_handle;

    const ::TF_MapOps* m_TF_MapOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
