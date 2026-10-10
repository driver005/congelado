// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/socket.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/socket.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_io_sonic:socket;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_SocketOps : public ice::sonic::Runtime<::TF_SocketOps, ::TF_Socket>
{
public:
    TF_SocketOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_SocketOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Socket* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_SocketOps(const ::TF_SocketOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_SocketOps(const ::TF_SocketOps* ops, ::TF_Socket* handle) noexcept :
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

    void close_socket() const noexcept
    {
        m_ops->close_socket(get_handle());
    }

    void set_non_blocking(int enabled, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_non_blocking(get_handle(), enabled, out_status.get_handle());
    }

    void set_reuse_address(int enabled, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_reuse_address(get_handle(), enabled, out_status.get_handle());
    }

    void set_broadcast(int enabled, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_broadcast(get_handle(), enabled, out_status.get_handle());
    }

    void set_tcp_no_delay(int enabled, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_tcp_no_delay(get_handle(), enabled, out_status.get_handle());
    }

    void load_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->load_certificate(
            get_handle(),
            cert_path.get_handle(),
            key_path.get_handle(),
            out_status.get_handle()
        );
    }

    void generate_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->generate_certificate(
            get_handle(),
            cert_path.get_handle(),
            key_path.get_handle(),
            out_status.get_handle()
        );
    }

    void set_verify_peer(int enabled, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_verify_peer(get_handle(), enabled, out_status.get_handle());
    }

    void bind(int allow_unauthorized, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->bind(get_handle(), allow_unauthorized, out_status.get_handle());
    }

    void listen(int backlog, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->listen(get_handle(), backlog, out_status.get_handle());
    }

    void join_multicast(
        const ice::sonic::String& group,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->join_multicast(get_handle(), group.get_handle(), out_status.get_handle());
    }

    void accept(
        const ice::sonic::TF_SocketOps& out_accepted,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->accept(get_handle(), out_accepted.get_handle(), out_status.get_handle());
    }

    void accept_async(
        TFSocketAcceptFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->accept_async(get_handle(), completion, user_data, out_status.get_handle());
    }

    void connect(int64_t timeout_ms, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->connect(get_handle(), timeout_ms, out_status.get_handle());
    }

    void connect_async(
        int64_t timeout_ms,
        TFSocketAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->connect_async(
            get_handle(),
            timeout_ms,
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void send(
        const void* data,
        size_t length,
        size_t* out_bytes_sent,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->send(get_handle(), data, length, out_bytes_sent, out_status.get_handle());
    }

    void send_async(
        const void* data,
        size_t length,
        TFSocketTransferFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->send_async(
            get_handle(),
            data,
            length,
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void receive(
        void* out_buffer,
        size_t buffer_size,
        size_t* out_bytes_received,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->receive(
            get_handle(),
            out_buffer,
            buffer_size,
            out_bytes_received,
            out_status.get_handle()
        );
    }

    void receive_async(
        void* out_buffer,
        size_t buffer_size,
        TFSocketTransferFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->receive_async(
            get_handle(),
            out_buffer,
            buffer_size,
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void send_to(
        const void* data,
        size_t length,
        const ice::sonic::String& dest_host,
        uint16_t dest_port,
        size_t* out_bytes_sent,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->send_to(
            get_handle(),
            data,
            length,
            dest_host.get_handle(),
            dest_port,
            out_bytes_sent,
            out_status.get_handle()
        );
    }

    void receive_from(
        void* out_buffer,
        size_t buffer_size,
        size_t* out_bytes_received,
        const ice::sonic::String& out_sender_host,
        uint16_t* out_sender_port,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->receive_from(
            get_handle(),
            out_buffer,
            buffer_size,
            out_bytes_received,
            out_sender_host.get_handle(),
            out_sender_port,
            out_status.get_handle()
        );
    }

    void set_send_timeout(int64_t timeout_ms, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set_send_timeout(get_handle(), timeout_ms, out_status.get_handle());
    }

    void set_receive_timeout(
        int64_t timeout_ms,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_receive_timeout(get_handle(), timeout_ms, out_status.get_handle());
    }

    void shutdown(int how, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->shutdown(get_handle(), how, out_status.get_handle());
    }

    void get_status(int* out_status) const noexcept
    {
        m_ops->get_status(get_handle(), out_status);
    }

    void get_error_code(int* out_error_code) const noexcept
    {
        m_ops->get_error_code(get_handle(), out_error_code);
    }

    void get_local_endpoint(
        const ice::sonic::String& out_host,
        uint16_t* out_port,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_local_endpoint(
            get_handle(),
            out_host.get_handle(),
            out_port,
            out_status.get_handle()
        );
    }

    void get_remote_endpoint(
        const ice::sonic::String& out_host,
        uint16_t* out_port,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_remote_endpoint(
            get_handle(),
            out_host.get_handle(),
            out_port,
            out_status.get_handle()
        );
    }

    void get_protocol(TFSocketProtocol* out_protocol) const noexcept
    {
        m_ops->get_protocol(get_handle(), out_protocol);
    }

    void get_fd(intptr_t* out_fd) const noexcept
    {
        m_ops->get_fd(get_handle(), out_fd);
    }

    void is_valid(int* out_valid) const noexcept
    {
        m_ops->is_valid(get_handle(), out_valid);
    }
};

} // namespace ice::sonic
