// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/request.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/request.h"

export module cc_abi_sonic_io;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_RequestOps : public ice::sonic::Runtime<TF_RequestOps, TF_RequestOps>
{
public:
    explicit TF_RequestOps(TF_RequestOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "io";

    [[nodiscard]] std::expected<void, ice::Status>
    set_method(const ice::sonic::TF_StringOps& method) noexcept
    {
        ice::Status status;
        m_ops->set_method(get_handle(), method.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_method(const ice::sonic::TF_StringOps& out_method) noexcept
    {
        ice::Status status;
        m_ops->get_method(get_handle(), out_method.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_path(const ice::sonic::TF_StringOps& path) noexcept
    {
        ice::Status status;
        m_ops->set_path(get_handle(), path.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_path(const ice::sonic::TF_StringOps& out_path) noexcept
    {
        ice::Status status;
        m_ops->get_path(get_handle(), out_path.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_scheme(const ice::sonic::TF_StringOps& scheme) noexcept
    {
        ice::Status status;
        m_ops->set_scheme(get_handle(), scheme.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_scheme(const ice::sonic::TF_StringOps& out_scheme) noexcept
    {
        ice::Status status;
        m_ops->get_scheme(get_handle(), out_scheme.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_authority(const ice::sonic::TF_StringOps& authority) noexcept
    {
        ice::Status status;
        m_ops->set_authority(get_handle(), authority.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_authority(const ice::sonic::TF_StringOps& out_authority) noexcept
    {
        ice::Status status;
        m_ops->get_authority(get_handle(), out_authority.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_header(const ice::sonic::TF_StringOps& name, const ice::sonic::TF_StringOps& value) noexcept
    {
        ice::Status status;
        m_ops->set_header(get_handle(), name.get_handle(), value.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_header(const ice::sonic::TF_StringOps& name, const ice::sonic::TF_StringOps& value) noexcept
    {
        ice::Status status;
        m_ops->add_header(get_handle(), name.get_handle(), value.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    remove_header(const ice::sonic::TF_StringOps& name) noexcept
    {
        ice::Status status;
        m_ops->remove_header(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    find_header(const ice::sonic::TF_StringOps& name, const TF_String** out_value) noexcept
    {
        ice::Status status;
        m_ops->find_header(get_handle(), name.get_handle(), out_value, status.get_handle());

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
    get_headers(const ice::sonic::TF_MapOps& out_headers) noexcept
    {
        ice::Status status;
        m_ops->get_headers(get_handle(), out_headers.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_query_param(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_StringOps& value
    ) noexcept
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
    get_query_params(const ice::sonic::TF_MapOps& out_params) noexcept
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
    set_content_type(const ice::sonic::TF_StringOps& content_type) noexcept
    {
        ice::Status status;
        m_ops->set_content_type(get_handle(), content_type.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_content_type(const ice::sonic::TF_StringOps& out_content_type) noexcept
    {
        ice::Status status;
        m_ops->get_content_type(get_handle(), out_content_type.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_accept(const ice::sonic::TF_StringOps& accept) noexcept
    {
        ice::Status status;
        m_ops->set_accept(get_handle(), accept.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_accept(const ice::sonic::TF_StringOps& out_accept) noexcept
    {
        ice::Status status;
        m_ops->get_accept(get_handle(), out_accept.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_user_agent(const ice::sonic::TF_StringOps& user_agent) noexcept
    {
        ice::Status status;
        m_ops->set_user_agent(get_handle(), user_agent.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_user_agent(const ice::sonic::TF_StringOps& out_user_agent) noexcept
    {
        ice::Status status;
        m_ops->get_user_agent(get_handle(), out_user_agent.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_bearer_auth(const ice::sonic::TF_StringOps& token) noexcept
    {
        ice::Status status;
        m_ops->set_bearer_auth(get_handle(), token.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_basic_auth(
        const ice::sonic::TF_StringOps& username,
        const ice::sonic::TF_StringOps& password
    ) noexcept
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

    [[nodiscard]] std::expected<void, ice::Status>
    get_authorization(const ice::sonic::TF_StringOps& out_authorization) noexcept
    {
        ice::Status status;
        m_ops->get_authorization(get_handle(), out_authorization.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_addr(const ice::sonic::TF_StringOps& addr) noexcept
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

    [[nodiscard]] std::expected<void, ice::Status> get_timeout(int64_t* out_timeout_ms) noexcept
    {
        ice::Status status;
        m_ops->get_timeout(get_handle(), out_timeout_ms, status.get_handle());

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

    [[nodiscard]] std::expected<void, ice::Status> get_stream_id(uint32_t* out_stream_id) noexcept
    {
        ice::Status status;
        m_ops->get_stream_id(get_handle(), out_stream_id, status.get_handle());

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
