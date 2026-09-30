// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/socket.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/socket.h"

export module cc_ice_extern_io_builder:socket;

import std;

export namespace ice::builder {

class TF_SocketOps
{
public:
    TF_SocketOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_SocketOps(const TF_SocketOps&) = delete;
    TF_SocketOps& operator=(const TF_SocketOps&) = delete;

    static TF_SocketOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_SocketOps*>(ctx);
    }

    template<typename HandleT>
    static TF_SocketOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_SocketOps*>(handle->plugin_data);
    }

    virtual ~TF_SocketOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> close_socket() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_non_blocking(int enabled) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_reuse_address(int enabled) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_broadcast(int enabled) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_tcp_no_delay(int enabled) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> load_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> generate_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_verify_peer(int enabled) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    bind(int allow_unauthorized) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> listen(int backlog) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    join_multicast(const ice::sonic::String& group) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    accept(const ice::sonic::TF_SocketOps& out_accepted) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    accept_async(TFSocketAcceptFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> connect(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    connect_async(int64_t timeout_ms, TFSocketAckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    send(const void* data, size_t length, size_t* out_bytes_sent) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> send_async(
        const void* data,
        size_t length,
        TFSocketTransferFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    receive(void* out_buffer, size_t buffer_size, size_t* out_bytes_received) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> receive_async(
        void* out_buffer,
        size_t buffer_size,
        TFSocketTransferFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> send_to(
        const void* data,
        size_t length,
        const ice::sonic::String& dest_host,
        uint16_t dest_port,
        size_t* out_bytes_sent
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> receive_from(
        void* out_buffer,
        size_t buffer_size,
        size_t* out_bytes_received,
        const ice::sonic::String& out_sender_host,
        uint16_t* out_sender_port
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_send_timeout(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_receive_timeout(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> shutdown(int how) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_status(int* out_status) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_error_code(int* out_error_code) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_local_endpoint(const ice::sonic::String& out_host, uint16_t* out_port) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_remote_endpoint(const ice::sonic::String& out_host, uint16_t* out_port) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_protocol(TFSocketProtocol* out_protocol) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_fd(intptr_t* out_fd) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> is_valid(int* out_valid) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_SocketOps{
            .struct_size = TF_SOCKET_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_SocketOps>{&TF_SocketOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_SocketOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .close_socket =
                [](TF_Socket* socket) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).close_socket();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_non_blocking =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).set_non_blocking(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_reuse_address =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).set_reuse_address(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_broadcast =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).set_broadcast(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_tcp_no_delay =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).set_tcp_no_delay(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .load_certificate =
                [](TF_Socket* socket,
                   const TF_String* cert_path,
                   const TF_String* key_path,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).load_certificate(
                    ice::sonic::String::wrap(cert_path),
                    ice::sonic::String::wrap(key_path)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .generate_certificate =
                [](TF_Socket* socket,
                   const TF_String* cert_path,
                   const TF_String* key_path,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).generate_certificate(
                    ice::sonic::String::wrap(cert_path),
                    ice::sonic::String::wrap(key_path)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_verify_peer =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).set_verify_peer(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .bind =
                [](TF_Socket* socket, int allow_unauthorized, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).bind(allow_unauthorized);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .listen =
                [](TF_Socket* socket, int backlog, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).listen(backlog);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .join_multicast =
                [](TF_Socket* socket, const TF_String* group, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).join_multicast(
                    ice::sonic::String::wrap(group)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .accept =
                [](TF_Socket* socket, TF_Socket* out_accepted, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).accept(
                    ice::sonic::TF_SocketOps::wrap(out_accepted)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .accept_async =
                [](TF_Socket* socket,
                   TFSocketAcceptFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).accept_async(completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .connect =
                [](TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).connect(timeout_ms);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .connect_async =
                [](TF_Socket* socket,
                   int64_t timeout_ms,
                   TFSocketAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket)
                               .connect_async(timeout_ms, completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .send =
                [](TF_Socket* socket,
                   const void* data,
                   size_t length,
                   size_t* out_bytes_sent,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).send(data, length, out_bytes_sent);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .send_async =
                [](TF_Socket* socket,
                   const void* data,
                   size_t length,
                   TFSocketTransferFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket)
                               .send_async(data, length, completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .receive =
                [](TF_Socket* socket,
                   void* out_buffer,
                   size_t buffer_size,
                   size_t* out_bytes_received,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket)
                               .receive(out_buffer, buffer_size, out_bytes_received);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .receive_async =
                [](TF_Socket* socket,
                   void* out_buffer,
                   size_t buffer_size,
                   TFSocketTransferFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket)
                               .receive_async(out_buffer, buffer_size, completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .send_to =
                [](TF_Socket* socket,
                   const void* data,
                   size_t length,
                   const TF_String* dest_host,
                   uint16_t dest_port,
                   size_t* out_bytes_sent,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).send_to(
                    data,
                    length,
                    ice::sonic::String::wrap(dest_host),
                    dest_port,
                    out_bytes_sent
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .receive_from =
                [](TF_Socket* socket,
                   void* out_buffer,
                   size_t buffer_size,
                   size_t* out_bytes_received,
                   TF_String* out_sender_host,
                   uint16_t* out_sender_port,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).receive_from(
                    out_buffer,
                    buffer_size,
                    out_bytes_received,
                    ice::sonic::String::wrap(out_sender_host),
                    out_sender_port
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_send_timeout =
                [](TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).set_send_timeout(timeout_ms);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_receive_timeout =
                [](TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).set_receive_timeout(timeout_ms);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .shutdown =
                [](TF_Socket* socket, int how, TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).shutdown(how);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_status =
                [](TF_Socket* socket, int* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).get_status(out_status);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_error_code =
                [](TF_Socket* socket, int* out_error_code) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).get_error_code(out_error_code);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_local_endpoint =
                [](TF_Socket* socket,
                   TF_String* out_host,
                   uint16_t* out_port,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).get_local_endpoint(
                    ice::sonic::String::wrap(out_host),
                    out_port
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_remote_endpoint =
                [](TF_Socket* socket,
                   TF_String* out_host,
                   uint16_t* out_port,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).get_remote_endpoint(
                    ice::sonic::String::wrap(out_host),
                    out_port
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_protocol =
                [](TF_Socket* socket, TFSocketProtocol* out_protocol) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).get_protocol(out_protocol);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_fd =
                [](TF_Socket* socket, intptr_t* out_fd) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).get_fd(out_fd);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_valid =
                [](TF_Socket* socket, int* out_valid) noexcept
            {
                auto res = TF_SocketOps::from_handle(socket).is_valid(out_valid);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_SocketOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Socket& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_SocketOps m_vtable;
    TF_Socket m_handle;
};

} // namespace ice::builder
