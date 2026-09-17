// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/client.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/client.h"

export module cc_abi_sonic_io;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ClientOps : public ice::sonic::Runtime<TF_ClientOps, TF_ClientOps>
{
public:
    explicit TF_ClientOps(TF_ClientOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "io";

    [[nodiscard]] std::expected<void, ice::Status> connect(int64_t timeout_ms) noexcept
    {
        ice::Status status;
        m_ops->connect(get_handle(), timeout_ms status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    connect_async(int64_t timeout_ms, TFClientConnectFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->connect_async(get_handle(), timeout_ms, completion, user_data status.get_handle());

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
    on_disconnect(TFClientDisconnectFn handler, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->on_disconnect(get_handle(), handler, user_data status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_connected(int* out_connected) noexcept
    {
        ice::Status status;
        m_ops->is_connected(get_handle(), out_connected status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_remote_endpoint(const ice::sonic::TF_StringOps& out_host, uint16_t* out_port) noexcept
    {
        ice::Status status;
        m_ops->get_remote_endpoint(
            get_handle(),
            out_host.get_handle(),
            out_port status.get_handle()
        );

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

    [[nodiscard]] std::expected<void, ice::Status>
    create_request(uint32_t stream_id, const ice::sonic::TF_RequestOps& out_request) noexcept
    {
        ice::Status status;
        m_ops
            ->create_request(get_handle(), stream_id, out_request.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> send(
        const ice::sonic::TF_RequestOps& request,
        const ice::sonic::TF_ResponseOps& out_response
    ) noexcept
    {
        ice::Status status;
        m_ops->send(
            get_handle(),
            request.get_handle(),
            out_response.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> send_async(
        const ice::sonic::TF_RequestOps& request,
        TFClientResponseFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->send_async(
            get_handle(),
            request.get_handle(),
            completion,
            user_data status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> cancel_request(uint32_t stream_id) noexcept
    {
        ice::Status status;
        m_ops->cancel_request(get_handle(), stream_id status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_pending_requests(const ice::sonic::TF_VectorOps& out_stream_ids) noexcept
    {
        ice::Status status;
        m_ops->list_pending_requests(get_handle(), out_stream_ids.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_pending_request_count(size_t* out_count) noexcept
    {
        ice::Status status;
        m_ops->get_pending_request_count(get_handle(), out_count status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    ping(TFClientConnectFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->ping(get_handle(), completion, user_data status.get_handle());

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
        m_ops->set_max_retries(get_handle(), max_retries status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_retry_backoff(int64_t backoff_ms) noexcept
    {
        ice::Status status;
        m_ops->set_retry_backoff(get_handle(), backoff_ms status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_timeout(int64_t timeout_ms) noexcept
    {
        ice::Status status;
        m_ops->set_timeout(get_handle(), timeout_ms status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    on_error(TFClientConnectFn handler, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->on_error(get_handle(), handler, user_data status.get_handle());

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
    get_stats(const ice::sonic::TF_MapOps& out_stats) noexcept
    {
        ice::Status status;
        m_ops->get_stats(get_handle(), out_stats.get_handle() status.get_handle());

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
