// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/response.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/response.h"

export module cc_abi_sonic_io;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ResponseOps : public ice::sonic::Runtime<TF_ResponseOps, TF_ResponseOps>
{
public:
    explicit TF_ResponseOps(TF_ResponseOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "io";

    [[nodiscard]] std::expected<void, ice::Status> set_status(int32_t status_code) noexcept
    {
        ice::Status status;
        m_ops->set_status(get_handle(), status_code status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_status(int32_t* out_status_code) noexcept
    {
        ice::Status status;
        m_ops->get_status(get_handle(), out_status_code status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_status_text(const ice::sonic::TF_StringOps& out_status_text) noexcept
    {
        ice::Status status;
        m_ops->get_status_text(get_handle(), out_status_text.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_header(const ice::sonic::TF_StringOps& name, const ice::sonic::TF_StringOps& value) noexcept
    {
        ice::Status status;
        m_ops->set_header(get_handle(), name.get_handle(), value.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_header(const ice::sonic::TF_StringOps& name, const ice::sonic::TF_StringOps& value) noexcept
    {
        ice::Status status;
        m_ops->add_header(get_handle(), name.get_handle(), value.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    remove_header(const ice::sonic::TF_StringOps& name) noexcept
    {
        ice::Status status;
        m_ops->remove_header(get_handle(), name.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    find_header(const ice::sonic::TF_StringOps& name, const TF_String** out_value) noexcept
    {
        ice::Status status;
        m_ops->find_header(get_handle(), name.get_handle(), out_value status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_headers(const ice::sonic::TF_MapOps& out_headers) noexcept
    {
        ice::Status status;
        m_ops->get_headers(get_handle(), out_headers.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_body(const void* data, size_t length) noexcept
    {
        ice::Status status;
        m_ops->set_body(get_handle(), data, length status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_body(const void** out_data, size_t* out_length) noexcept
    {
        ice::Status status;
        m_ops->get_body(get_handle(), out_data, out_length status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_keep_alive(int enabled) noexcept
    {
        ice::Status status;
        m_ops->set_keep_alive(get_handle(), enabled status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_keep_alive(int* out_keep_alive) noexcept
    {
        ice::Status status;
        m_ops->is_keep_alive(get_handle(), out_keep_alive status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_cookie(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_StringOps& value,
        const ice::sonic::TF_MapOps& attributes
    ) noexcept
    {
        ice::Status status;
        m_ops->set_cookie(
            get_handle(),
            name.get_handle(),
            value.get_handle(),
            attributes.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_set_cookies(const ice::sonic::TF_VectorOps& out_cookies) noexcept
    {
        ice::Status status;
        m_ops->get_set_cookies(get_handle(), out_cookies.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_content_type(const ice::sonic::TF_StringOps& out_content_type) noexcept
    {
        ice::Status status;
        m_ops->get_content_type(get_handle(), out_content_type.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_content_length(int64_t* out_length) noexcept
    {
        ice::Status status;
        m_ops->get_content_length(get_handle(), out_length status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_location(const ice::sonic::TF_StringOps& out_location) noexcept
    {
        ice::Status status;
        m_ops->get_location(get_handle(), out_location.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_etag(const ice::sonic::TF_StringOps& out_etag) noexcept
    {
        ice::Status status;
        m_ops->get_etag(get_handle(), out_etag.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_date(const ice::sonic::TF_StringOps& out_date) noexcept
    {
        ice::Status status;
        m_ops->get_date(get_handle(), out_date.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_server(const ice::sonic::TF_StringOps& out_server) noexcept
    {
        ice::Status status;
        m_ops->get_server(get_handle(), out_server.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_cache_control(const ice::sonic::TF_StringOps& out_cache_control) noexcept
    {
        ice::Status status;
        m_ops->get_cache_control(get_handle(), out_cache_control.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_last_modified(const ice::sonic::TF_StringOps& out_last_modified) noexcept
    {
        ice::Status status;
        m_ops->get_last_modified(get_handle(), out_last_modified.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_informational(int* out_result) noexcept
    {
        ice::Status status;
        m_ops->is_informational(get_handle(), out_result status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_success(int* out_result) noexcept
    {
        ice::Status status;
        m_ops->is_success(get_handle(), out_result status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_redirection(int* out_result) noexcept
    {
        ice::Status status;
        m_ops->is_redirection(get_handle(), out_result status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_client_error(int* out_result) noexcept
    {
        ice::Status status;
        m_ops->is_client_error(get_handle(), out_result status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_server_error(int* out_result) noexcept
    {
        ice::Status status;
        m_ops->is_server_error(get_handle(), out_result status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
