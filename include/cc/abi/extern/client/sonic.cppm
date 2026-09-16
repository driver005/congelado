// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/client/client.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/client/client.h"

export module cc_abi_sonic_client;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Client : public ice::sonic::Runtime<Client, TF_Client>
{
public:
    explicit Client(TF_Client* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "client";

    [[nodiscard]] std::expected<void, ice::Status>
    new_client(const ice::sonic::String& host, uint16_t port) noexcept
    {
        ice::Status status;
        m_ops->new_client(get_handle(), host.get_handle(), port, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> destroy_client() noexcept
    {
        ice::Status status;
        m_ops->destroy_client(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> connect(int64_t timeout_ms) noexcept
    {
        ice::Status status;
        m_ops->connect(get_handle(), timeout_ms, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    connect_async(int64_t timeout_ms, TF_Client_ConnectFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->connect_async(get_handle(), timeout_ms, completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> disconnect() noexcept
    {
        ice::Status status;
        m_ops->disconnect(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> reconnect() noexcept
    {
        ice::Status status;
        m_ops->reconnect(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    on_disconnect(TF_Client_DisconnectFn handler, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->on_disconnect(get_handle(), handler, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_connected() noexcept
    {
        ice::Status status;
        m_ops->is_connected(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_remote_endpoint(TF_String* out_host, uint16_t* out_port) noexcept
    {
        ice::Status status;
        m_ops->get_remote_endpoint(get_handle(), out_host, out_port, status.get_handle());

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

    [[nodiscard]] std::expected<void, ice::Status> create_request(uint32_t stream_id) noexcept
    {
        ice::Status status;
        m_ops->create_request(get_handle(), stream_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    send(const ice::sonic::Request& request, const ice::sonic::Response& out_response) noexcept
    {
        ice::Status status;
        m_ops->send(
            get_handle(),
            request.get_handle(),
            out_response.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> send_async(
        const ice::sonic::Request& request,
        TF_Client_ResponseFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->send_async(
            get_handle(),
            request.get_handle(),
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> cancel_request(uint32_t stream_id) noexcept
    {
        ice::Status status;
        m_ops->cancel_request(get_handle(), stream_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_pending_requests(const ice::sonic::Vector& out_stream_ids) noexcept
    {
        ice::Status status;
        m_ops
            ->list_pending_requests(get_handle(), out_stream_ids.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_pending_request_count() noexcept
    {
        ice::Status status;
        m_ops->get_pending_request_count(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    ping(TF_Client_ConnectFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->ping(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> retry() noexcept
    {
        ice::Status status;
        m_ops->retry(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_max_retries(int max_retries) noexcept
    {
        ice::Status status;
        m_ops->set_max_retries(get_handle(), max_retries, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_retry_backoff(int64_t backoff_ms) noexcept
    {
        ice::Status status;
        m_ops->set_retry_backoff(get_handle(), backoff_ms, status.get_handle());

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

    [[nodiscard]] std::expected<void, ice::Status>
    on_error(TF_Client_ConnectFn handler, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->on_error(get_handle(), handler, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_last_error() noexcept
    {
        ice::Status status;
        m_ops->get_last_error(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_stats(const ice::sonic::Map& out_stats) noexcept
    {
        ice::Status status;
        m_ops->get_stats(get_handle(), out_stats.get_handle(), status.get_handle());

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
