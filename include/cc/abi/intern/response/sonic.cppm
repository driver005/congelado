// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/response/response.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/response/response.h"

export module cc_abi_sonic_response;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Response : public ice::sonic::Runtime<Response, TF_Response>
{
public:
    explicit Response(TF_Response* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "response";

    [[nodiscard]] std::expected<void, ice::Status> new_response(uint32_t stream_id) noexcept
    {
        ice::Status status;
        m_ops->new_response(get_handle(), stream_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> destroy_response() noexcept
    {
        ice::Status status;
        m_ops->destroy_response(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_status(int32_t status_code) noexcept
    {
        ice::Status status;
        m_ops->set_status(get_handle(), status_code, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_status() noexcept
    {
        ice::Status status;
        m_ops->get_status(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_status_text(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_status_text(get_handle(), out, status.get_handle());

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

    [[nodiscard]] std::expected<void, ice::Status> set_keep_alive(int enabled) noexcept
    {
        ice::Status status;
        m_ops->set_keep_alive(get_handle(), enabled, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_keep_alive() noexcept
    {
        ice::Status status;
        m_ops->is_keep_alive(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_cookie(
        const ice::sonic::String& name,
        const ice::sonic::String& value,
        const ice::sonic::Map& attributes
    ) noexcept
    {
        ice::Status status;
        m_ops->set_cookie(
            get_handle(),
            name.get_handle(),
            value.get_handle(),
            attributes.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_set_cookies(const ice::sonic::Vector& out_cookies) noexcept
    {
        ice::Status status;
        m_ops->get_set_cookies(get_handle(), out_cookies.get_handle(), status.get_handle());

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

    [[nodiscard]] std::expected<void, ice::Status> get_content_length() noexcept
    {
        ice::Status status;
        m_ops->get_content_length(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_location(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_location(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_etag(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_etag(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_date(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_date(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_server(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_server(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_cache_control(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_cache_control(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_last_modified(TF_String* out) noexcept
    {
        ice::Status status;
        m_ops->get_last_modified(get_handle(), out, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_informational() noexcept
    {
        ice::Status status;
        m_ops->is_informational(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_success() noexcept
    {
        ice::Status status;
        m_ops->is_success(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_redirection() noexcept
    {
        ice::Status status;
        m_ops->is_redirection(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_client_error() noexcept
    {
        ice::Status status;
        m_ops->is_client_error(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_server_error() noexcept
    {
        ice::Status status;
        m_ops->is_server_error(get_handle(), status.get_handle());

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
