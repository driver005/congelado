// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/request/request.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/request/request.h"

export module cc_abi_sonic_request;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Request : public ice::sonic::Runtime<Request, TF_Request>
{
public:
    explicit Request(TF_Request* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "request";

    [[nodiscard]] std::expected<void, ice::Status> new_request(uint32_t stream_id) noexcept
    {
        ice::Status status;
        m_ops->new_request(get_handle(), stream_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> destroy_request() noexcept
    {
        ice::Status status;
        m_ops->destroy_request(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_method(const ice::sonic::String& method) noexcept
    {
        ice::Status status;
        m_ops->set_method(get_handle(), method.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_method(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_method(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_path(const ice::sonic::String& path) noexcept
    {
        ice::Status status;
        m_ops->set_path(get_handle(), path.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_path(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_path(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_scheme(const ice::sonic::String& scheme) noexcept
    {
        ice::Status status;
        m_ops->set_scheme(get_handle(), scheme.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_scheme(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_scheme(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_authority(const ice::sonic::String& authority) noexcept
    {
        ice::Status status;
        m_ops->set_authority(get_handle(), authority.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_authority(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_authority(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_header(const ice::sonic::String& name, const ice::sonic::String& value) noexcept
    {
        ice::Status status;
        m_ops->set_header(get_handle(), name.get_handle(), value.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_header(const ice::sonic::String& name, const ice::sonic::String& value) noexcept
    {
        ice::Status status;
        m_ops->add_header(get_handle(), name.get_handle(), value.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    remove_header(const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->remove_header(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    find_header(const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->find_header(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> clear_headers() noexcept
    {
        ice::Status status;
        m_ops->clear_headers(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_headers(const ice::sonic::Map& out_headers) noexcept
    {
        ice::Status status;
        m_ops->get_headers(get_handle(), out_headers.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_query_param(const ice::sonic::String& name, const ice::sonic::String& value) noexcept
    {
        ice::Status status;
        m_ops->set_query_param(
            get_handle(),
            name.get_handle(),
            value.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_query_params(const ice::sonic::Map& out_params) noexcept
    {
        ice::Status status;
        m_ops->get_query_params(get_handle(), out_params.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_body(const void* data, size_t length) noexcept
    {
        ice::Status status;
        m_ops->set_body(get_handle(), data, length, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_body(const void** out_data, size_t* out_length) noexcept
    {
        ice::Status status;
        m_ops->get_body(get_handle(), out_data, out_length, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_content_type(const ice::sonic::String& content_type) noexcept
    {
        ice::Status status;
        m_ops->set_content_type(get_handle(), content_type.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_content_type(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_content_type(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_accept(const ice::sonic::String& accept) noexcept
    {
        ice::Status status;
        m_ops->set_accept(get_handle(), accept.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_accept(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_accept(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_user_agent(const ice::sonic::String& user_agent) noexcept
    {
        ice::Status status;
        m_ops->set_user_agent(get_handle(), user_agent.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_user_agent(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_user_agent(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_bearer_auth(const ice::sonic::String& token) noexcept
    {
        ice::Status status;
        m_ops->set_bearer_auth(get_handle(), token.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_basic_auth(const ice::sonic::String& username, const ice::sonic::String& password) noexcept
    {
        ice::Status status;
        m_ops->set_basic_auth(
            get_handle(),
            username.get_handle(),
            password.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_authorization(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_authorization(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_addr(const ice::sonic::String& addr) noexcept
    {
        ice::Status status;
        m_ops->set_addr(get_handle(), addr.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_no_decompress(int enabled) noexcept
    {
        ice::Status status;
        m_ops->set_no_decompress(get_handle(), enabled, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_timeout(int64_t timeout_ms) noexcept
    {
        ice::Status status;
        m_ops->set_timeout(get_handle(), timeout_ms, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_timeout() noexcept
    {
        ice::Status status;
        m_ops->get_timeout(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_stream_id(uint32_t stream_id) noexcept
    {
        ice::Status status;
        m_ops->set_stream_id(get_handle(), stream_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_stream_id() noexcept
    {
        ice::Status status;
        m_ops->get_stream_id(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
