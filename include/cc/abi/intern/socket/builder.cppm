// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/socket/socket.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/socket/socket.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_socket;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Socket
{
public:
    static Socket* create(void* ctx) noexcept
    {
        return static_cast<Socket*>(ctx);
    }

    template<typename HandleT>
    static Socket* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Socket*>(handle);
    }

    virtual ~Socket() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_socket(
        TF_Socket_Protocol protocol,
        const ice::sonic::String& host,
        uint16_t port
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> close_socket() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_non_blocking(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_reuse_address(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_broadcast(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_tcp_no_delay(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> load_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> generate_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_verify_peer(int enabled) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> bind(int allow_unauthorized) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> listen(int backlog) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    join_multicast(const ice::sonic::String& group) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    accept(TF_Socket_Handle** out_accepted) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    accept_async(TF_Socket_AcceptFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> connect(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    connect_async(int64_t timeout_ms, TF_Socket_AckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    send(const void* data, size_t length, size_t* out_bytes_sent) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> send_async(
        const void* data,
        size_t length,
        TF_Socket_TransferFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    receive(void* out_buffer, size_t buffer_size, size_t* out_bytes_received) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> receive_async(
        void* out_buffer,
        size_t buffer_size,
        TF_Socket_TransferFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> send_to(
        const void* data,
        size_t length,
        const ice::sonic::String& dest_host,
        uint16_t dest_port,
        size_t* out_bytes_sent
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> receive_from(
        void* out_buffer,
        size_t buffer_size,
        size_t* out_bytes_received,
        TF_String* out_sender_host,
        uint16_t* out_sender_port
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_send_timeout(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_receive_timeout(int64_t timeout_ms) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> shutdown(int how) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_status() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_error_code() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_local_endpoint(TF_String* out_host, uint16_t* out_port) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_remote_endpoint(TF_String* out_host, uint16_t* out_port) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_protocol() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_fd() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> is_valid() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Socket* get_generic_vtable()
    {
        static TF_Socket vtable = {
            .struct_size = TF_SOCKET_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Socket::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Socket::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .new_socket =
                [](void* plugin_context,
                   TF_Socket_Protocol protocol,
                   const TF_String_Handle* host,
                   uint16_t port,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(plugin_context);
                auto res = self->new_socket(protocol, ice::sonic::String::wrap(host), port);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .close_socket =
                [](TF_Socket_Handle* socket) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->close_socket();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_non_blocking =
                [](TF_Socket_Handle* socket, int enabled, TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->set_non_blocking(enabled);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_reuse_address =
                [](TF_Socket_Handle* socket, int enabled, TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->set_reuse_address(enabled);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_broadcast =
                [](TF_Socket_Handle* socket, int enabled, TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->set_broadcast(enabled);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_tcp_no_delay =
                [](TF_Socket_Handle* socket, int enabled, TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->set_tcp_no_delay(enabled);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .load_certificate =
                [](TF_Socket_Handle* socket,
                   const TF_String_Handle* cert_path,
                   const TF_String_Handle* key_path,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->load_certificate(
                    ice::sonic::String::wrap(cert_path),
                    ice::sonic::String::wrap(key_path)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .generate_certificate =
                [](void* plugin_context,
                   const TF_String_Handle* cert_path,
                   const TF_String_Handle* key_path,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(plugin_context);
                auto res = self->generate_certificate(
                    ice::sonic::String::wrap(cert_path),
                    ice::sonic::String::wrap(key_path)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_verify_peer =
                [](TF_Socket_Handle* socket, int enabled, TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->set_verify_peer(enabled);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .bind =
                [](TF_Socket_Handle* socket,
                   int allow_unauthorized,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->bind(allow_unauthorized);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .listen =
                [](TF_Socket_Handle* socket, int backlog, TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->listen(backlog);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .join_multicast =
                [](TF_Socket_Handle* socket,
                   const TF_String_Handle* group,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->join_multicast(ice::sonic::String::wrap(group));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .accept =
                [](TF_Socket_Handle* socket,
                   TF_Socket_Handle** out_accepted,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->accept(out_accepted);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .accept_async =
                [](TF_Socket_Handle* socket,
                   TF_Socket_AcceptFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->accept_async(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .connect =
                [](TF_Socket_Handle* socket, int64_t timeout_ms, TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->connect(timeout_ms);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .connect_async =
                [](TF_Socket_Handle* socket,
                   int64_t timeout_ms,
                   TF_Socket_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->connect_async(timeout_ms, completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .send =
                [](TF_Socket_Handle* socket,
                   const void* data,
                   size_t length,
                   size_t* out_bytes_sent,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->send(data, length, out_bytes_sent);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .send_async =
                [](TF_Socket_Handle* socket,
                   const void* data,
                   size_t length,
                   TF_Socket_TransferFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->send_async(data, length, completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .receive =
                [](TF_Socket_Handle* socket,
                   void* out_buffer,
                   size_t buffer_size,
                   size_t* out_bytes_received,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->receive(out_buffer, buffer_size, out_bytes_received);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .receive_async =
                [](TF_Socket_Handle* socket,
                   void* out_buffer,
                   size_t buffer_size,
                   TF_Socket_TransferFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->receive_async(out_buffer, buffer_size, completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .send_to =
                [](TF_Socket_Handle* socket,
                   const void* data,
                   size_t length,
                   const TF_String_Handle* dest_host,
                   uint16_t dest_port,
                   size_t* out_bytes_sent,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->send_to(
                    data,
                    length,
                    ice::sonic::String::wrap(dest_host),
                    dest_port,
                    out_bytes_sent
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .receive_from =
                [](TF_Socket_Handle* socket,
                   void* out_buffer,
                   size_t buffer_size,
                   size_t* out_bytes_received,
                   TF_String* out_sender_host,
                   uint16_t* out_sender_port,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->receive_from(
                    out_buffer,
                    buffer_size,
                    out_bytes_received,
                    out_sender_host,
                    out_sender_port
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_send_timeout =
                [](TF_Socket_Handle* socket, int64_t timeout_ms, TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->set_send_timeout(timeout_ms);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_receive_timeout =
                [](TF_Socket_Handle* socket, int64_t timeout_ms, TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->set_receive_timeout(timeout_ms);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .shutdown =
                [](TF_Socket_Handle* socket, int how, TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->shutdown(how);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_status =
                [](TF_Socket_Handle* socket) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->get_status();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_error_code =
                [](TF_Socket_Handle* socket) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->get_error_code();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_local_endpoint =
                [](TF_Socket_Handle* socket,
                   TF_String* out_host,
                   uint16_t* out_port,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->get_local_endpoint(out_host, out_port);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_remote_endpoint =
                [](TF_Socket_Handle* socket,
                   TF_String* out_host,
                   uint16_t* out_port,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->get_remote_endpoint(out_host, out_port);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_protocol =
                [](TF_Socket_Handle* socket) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->get_protocol();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_fd =
                [](TF_Socket_Handle* socket) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->get_fd();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_valid =
                [](TF_Socket_Handle* socket) noexcept
            {
                auto* self = Socket::create(socket);
                auto res = self->is_valid();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
