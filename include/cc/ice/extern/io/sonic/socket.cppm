// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/socket.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/socket.h"

export module cc_ice_extern_io_sonic:socket;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_SocketOps : public ice::sonic::Runtime<TF_SocketOps, TF_SocketOps>
{
public:
    explicit TF_SocketOps(TF_SocketOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "io";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void close_socket() noexcept
    {
        m_ops->close_socket(get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> set_non_blocking(int enabled) noexcept
    {
        ice::sonic::Status status;
        m_ops->set_non_blocking(get_handle(), enabled, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> set_reuse_address(int enabled) noexcept
    {
        ice::sonic::Status status;
        m_ops->set_reuse_address(get_handle(), enabled, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> set_broadcast(int enabled) noexcept
    {
        ice::sonic::Status status;
        m_ops->set_broadcast(get_handle(), enabled, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> set_tcp_no_delay(int enabled) noexcept
    {
        ice::sonic::Status status;
        m_ops->set_tcp_no_delay(get_handle(), enabled, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> load_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->load_certificate(
            get_handle(),
            cert_path.get_handle(),
            key_path.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> generate_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->generate_certificate(
            get_handle(),
            cert_path.get_handle(),
            key_path.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> set_verify_peer(int enabled) noexcept
    {
        ice::sonic::Status status;
        m_ops->set_verify_peer(get_handle(), enabled, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> bind(int allow_unauthorized) noexcept
    {
        ice::sonic::Status status;
        m_ops->bind(get_handle(), allow_unauthorized, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> listen(int backlog) noexcept
    {
        ice::sonic::Status status;
        m_ops->listen(get_handle(), backlog, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    join_multicast(const ice::sonic::String& group) noexcept
    {
        ice::sonic::Status status;
        m_ops->join_multicast(get_handle(), group.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    accept(const ice::sonic::TF_SocketOps& out_accepted) noexcept
    {
        ice::sonic::Status status;
        m_ops->accept(get_handle(), out_accepted.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    accept_async(TFSocketAcceptFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->accept_async(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> connect(int64_t timeout_ms) noexcept
    {
        ice::sonic::Status status;
        m_ops->connect(get_handle(), timeout_ms, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    connect_async(int64_t timeout_ms, TFSocketAckFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->connect_async(get_handle(), timeout_ms, completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    send(const void* data, size_t length, size_t* out_bytes_sent) noexcept
    {
        ice::sonic::Status status;
        m_ops->send(get_handle(), data, length, out_bytes_sent, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> send_async(
        const void* data,
        size_t length,
        TFSocketTransferFn completion,
        void* user_data
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->send_async(get_handle(), data, length, completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    receive(void* out_buffer, size_t buffer_size, size_t* out_bytes_received) noexcept
    {
        ice::sonic::Status status;
        m_ops->receive(
            get_handle(),
            out_buffer,
            buffer_size,
            out_bytes_received,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> receive_async(
        void* out_buffer,
        size_t buffer_size,
        TFSocketTransferFn completion,
        void* user_data
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->receive_async(
            get_handle(),
            out_buffer,
            buffer_size,
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> send_to(
        const void* data,
        size_t length,
        const ice::sonic::String& dest_host,
        uint16_t dest_port,
        size_t* out_bytes_sent
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->send_to(
            get_handle(),
            data,
            length,
            dest_host.get_handle(),
            dest_port,
            out_bytes_sent,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> receive_from(
        void* out_buffer,
        size_t buffer_size,
        size_t* out_bytes_received,
        const ice::sonic::String& out_sender_host,
        uint16_t* out_sender_port
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->receive_from(
            get_handle(),
            out_buffer,
            buffer_size,
            out_bytes_received,
            out_sender_host.get_handle(),
            out_sender_port,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set_send_timeout(int64_t timeout_ms) noexcept
    {
        ice::sonic::Status status;
        m_ops->set_send_timeout(get_handle(), timeout_ms, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set_receive_timeout(int64_t timeout_ms) noexcept
    {
        ice::sonic::Status status;
        m_ops->set_receive_timeout(get_handle(), timeout_ms, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> shutdown(int how) noexcept
    {
        ice::sonic::Status status;
        m_ops->shutdown(get_handle(), how, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void get_status(int* out_status) noexcept
    {
        m_ops->get_status(get_handle(), out_status);
    }

    void get_error_code(int* out_error_code) noexcept
    {
        m_ops->get_error_code(get_handle(), out_error_code);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_local_endpoint(const ice::sonic::String& out_host, uint16_t* out_port) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_local_endpoint(
            get_handle(),
            out_host.get_handle(),
            out_port,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_remote_endpoint(const ice::sonic::String& out_host, uint16_t* out_port) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_remote_endpoint(
            get_handle(),
            out_host.get_handle(),
            out_port,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void get_protocol(TFSocketProtocol* out_protocol) noexcept
    {
        m_ops->get_protocol(get_handle(), out_protocol);
    }

    void get_fd(intptr_t* out_fd) noexcept
    {
        m_ops->get_fd(get_handle(), out_fd);
    }

    void is_valid(int* out_valid) noexcept
    {
        m_ops->is_valid(get_handle(), out_valid);
    }
};

} // namespace ice::sonic
