// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/socket.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/socket.h"

export module cc_abi_builder_io;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_SocketOps
{
public:
    static TF_SocketOps* create(void* ctx) noexcept
    {
        return static_cast<TF_SocketOps*>(ctx);
    }

    template<typename HandleT>
    static TF_SocketOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_SocketOps*>(handle->plugin_data);
    }

    virtual ~TF_SocketOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> close_socket() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_non_blocking(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_reuse_address(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_broadcast(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_tcp_no_delay(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> load_certificate(
        const ice::sonic::TF_StringOps& cert_path,
        const ice::sonic::TF_StringOps& key_path
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> generate_certificate(
        const ice::sonic::TF_StringOps& cert_path,
        const ice::sonic::TF_StringOps& key_path
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_verify_peer(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> bind(int allow_unauthorized) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> listen(int backlog) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    join_multicast(const ice::sonic::TF_StringOps& group) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    accept(const ice::sonic::TF_SocketOps& out_accepted) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    accept_async(TFSocketAcceptFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> connect(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    connect_async(int64_t timeout_ms, TFSocketAckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    send(const void* data, size_t length, size_t* out_bytes_sent) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> send_async(
        const void* data,
        size_t length,
        TFSocketTransferFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    receive(void* out_buffer, size_t buffer_size, size_t* out_bytes_received) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> receive_async(
        void* out_buffer,
        size_t buffer_size,
        TFSocketTransferFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> send_to(
        const void* data,
        size_t length,
        const ice::sonic::TF_StringOps& dest_host,
        uint16_t dest_port,
        size_t* out_bytes_sent
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> receive_from(
        void* out_buffer,
        size_t buffer_size,
        size_t* out_bytes_received,
        const ice::sonic::TF_StringOps& out_sender_host,
        uint16_t* out_sender_port
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_send_timeout(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_receive_timeout(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> shutdown(int how) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_status(int* out_status) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_error_code(int* out_error_code) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_local_endpoint(const ice::sonic::TF_StringOps& out_host, uint16_t* out_port) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_remote_endpoint(const ice::sonic::TF_StringOps& out_host, uint16_t* out_port) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_protocol(TFSocketProtocol* out_protocol) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_fd(intptr_t* out_fd) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_valid(int* out_valid) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_SocketOps* get_generic_vtable()
    {
        static TF_SocketOps vtable = {
            .struct_size = TF_SOCKET_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_SocketOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_SocketOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .close_socket =
                [](TF_Socket* socket) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->close_socket();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_non_blocking =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->set_non_blocking(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_reuse_address =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->set_reuse_address(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_broadcast =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->set_broadcast(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_tcp_no_delay =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->set_tcp_no_delay(enabled);
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
                auto* self = TF_SocketOps::create(socket);
                auto res = self->load_certificate(
                    ice::sonic::TF_StringOps::wrap(cert_path),
                    ice::sonic::TF_StringOps::wrap(key_path)
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
                auto* self = TF_SocketOps::create(socket);
                auto res = self->generate_certificate(
                    ice::sonic::TF_StringOps::wrap(cert_path),
                    ice::sonic::TF_StringOps::wrap(key_path)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_verify_peer =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->set_verify_peer(enabled);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .bind =
                [](TF_Socket* socket, int allow_unauthorized, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->bind(allow_unauthorized);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .listen =
                [](TF_Socket* socket, int backlog, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->listen(backlog);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .join_multicast =
                [](TF_Socket* socket, const TF_String* group, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->join_multicast(ice::sonic::TF_StringOps::wrap(group));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .accept =
                [](TF_Socket* socket, TF_Socket* out_accepted, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->accept(ice::sonic::TF_SocketOps::wrap(out_accepted));
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
                auto* self = TF_SocketOps::create(socket);
                auto res = self->accept_async(completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .connect =
                [](TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->connect(timeout_ms);
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
                auto* self = TF_SocketOps::create(socket);
                auto res = self->connect_async(timeout_ms, completion, user_data);
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
                auto* self = TF_SocketOps::create(socket);
                auto res = self->send(data, length, out_bytes_sent);
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
                auto* self = TF_SocketOps::create(socket);
                auto res = self->send_async(data, length, completion, user_data);
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
                auto* self = TF_SocketOps::create(socket);
                auto res = self->receive(out_buffer, buffer_size, out_bytes_received);
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
                auto* self = TF_SocketOps::create(socket);
                auto res = self->receive_async(out_buffer, buffer_size, completion, user_data);
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
                auto* self = TF_SocketOps::create(socket);
                auto res = self->send_to(
                    data,
                    length,
                    ice::sonic::TF_StringOps::wrap(dest_host),
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
                auto* self = TF_SocketOps::create(socket);
                auto res = self->receive_from(
                    out_buffer,
                    buffer_size,
                    out_bytes_received,
                    ice::sonic::TF_StringOps::wrap(out_sender_host),
                    out_sender_port
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_send_timeout =
                [](TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->set_send_timeout(timeout_ms);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_receive_timeout =
                [](TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->set_receive_timeout(timeout_ms);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .shutdown =
                [](TF_Socket* socket, int how, TF_Status* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->shutdown(how);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_status =
                [](TF_Socket* socket, int* out_status) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->get_status(out_status);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_error_code =
                [](TF_Socket* socket, int* out_error_code) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->get_error_code(out_error_code);
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
                auto* self = TF_SocketOps::create(socket);
                auto res =
                    self->get_local_endpoint(ice::sonic::TF_StringOps::wrap(out_host), out_port);
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
                auto* self = TF_SocketOps::create(socket);
                auto res =
                    self->get_remote_endpoint(ice::sonic::TF_StringOps::wrap(out_host), out_port);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_protocol =
                [](TF_Socket* socket, TFSocketProtocol* out_protocol) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->get_protocol(out_protocol);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_fd =
                [](TF_Socket* socket, intptr_t* out_fd) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->get_fd(out_fd);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_valid =
                [](TF_Socket* socket, int* out_valid) noexcept
            {
                auto* self = TF_SocketOps::create(socket);
                auto res = self->is_valid(out_valid);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
