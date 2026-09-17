#ifndef TENSORFLOW_C_EXTERN_SOCKET_H_
#define TENSORFLOW_C_EXTERN_SOCKET_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Socket — generic transport socket, covering TCP/UDP/TLS/QUIC uniformly. Mirrors io::base::socket::Socket<Protocol> directly, with the protocol chosen at construction (a runtime enum here, since C has no template parameter) rather than reinventing a narrower design.
    typedef enum TFSocketProtocol
    {
        TF_SOCKET_TCP = 0,
        TF_SOCKET_UDP = 1,
        TF_SOCKET_TLS = 2,
        TF_SOCKET_QUIC = 3
    } TFSocketProtocol;

    typedef enum TFSocketStatus
    {
        TF_SOCKET_ERRORED = 0,
        TF_SOCKET_VALID = 1,
        TF_SOCKET_CLEANLY_DISCONNECTED = 2,
        TF_SOCKET_WOULD_BLOCK = 3,
        TF_SOCKET_TIMED_OUT = 4
    } TFSocketStatus;

    typedef struct TF_Socket
    {
        void* plugin_data;
    } TF_Socket;
    typedef void (*TFSocketAckFn)(void* user_data, TF_Status* out_status);
    typedef void (*TFSocketAcceptFn)(void* user_data, TF_Socket* accepted, TF_Status* out_status);
    typedef void (*TFSocketTransferFn)(void* user_data, size_t bytes, TF_Status* out_status);

    // Plugin-facing vtable registered via create_socket.
    typedef struct TF_SocketOps
    {
        size_t struct_size;

        void (*destroy)(TF_Socket* socket);
        void (*get_name)(TF_Socket* socket, TF_String* out_name);

        void (*close_socket)(TF_Socket* socket);

        // Options.
        void (*set_non_blocking)(TF_Socket* socket, int enabled, TF_Status* out_status);
        void (*set_reuse_address)(TF_Socket* socket, int enabled, TF_Status* out_status);
        void (*set_broadcast)(TF_Socket* socket, int enabled, TF_Status* out_status);

        // No-op for UDP.
        void (*set_tcp_no_delay)(TF_Socket* socket, int enabled, TF_Status* out_status);

        // TLS (TF_SOCKET_TLS/TF_SOCKET_QUIC only).
        void (*load_certificate)(TF_Socket* socket, const TF_String* cert_path, const TF_String* key_path, TF_Status* out_status);
        void (*generate_certificate)(TF_Socket* socket, const TF_String* cert_path, const TF_String* key_path, TF_Status* out_status);
        void (*set_verify_peer)(TF_Socket* socket, int enabled, TF_Status* out_status);

        // Server-side lifecycle.
        void (*bind)(TF_Socket* socket, int allow_unauthorized, TF_Status* out_status);
        void (*listen)(TF_Socket* socket, int backlog, TF_Status* out_status);

        // UDP only.
        void (*join_multicast)(TF_Socket* socket, const TF_String* group, TF_Status* out_status);

        void (*accept)(TF_Socket* socket, TF_Socket* out_accepted, TF_Status* out_status);
        void (*accept_async)(TF_Socket* socket, TFSocketAcceptFn completion, void* user_data, TF_Status* out_status);

        // Client-side lifecycle.
        void (*connect)(TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status);
        void (*connect_async)(TF_Socket* socket, int64_t timeout_ms, TFSocketAckFn completion, void* user_data, TF_Status* out_status);

        // Data transfer (connected sockets: TCP/TLS/QUIC, or a UDP socket that has itself called connect()).
        void (*send)(TF_Socket* socket, const void* data, size_t length, size_t* out_bytes_sent, TF_Status* out_status);
        void (*send_async)(
            TF_Socket* socket,
            const void* data,
            size_t length,
            TFSocketTransferFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*receive)(TF_Socket* socket, void* out_buffer, size_t buffer_size, size_t* out_bytes_received, TF_Status* out_status);
        void (*receive_async)(
            TF_Socket* socket,
            void* out_buffer,
            size_t buffer_size,
            TFSocketTransferFn completion,
            void* user_data,
            TF_Status* out_status
        );

        // Connectionless data transfer (UDP without connect()) — explicit per-datagram destination/sender, matching sendto/recvfrom.
        void (*send_to)(
            TF_Socket* socket,
            const void* data,
            size_t length,
            const TF_String* dest_host,
            uint16_t dest_port,
            size_t* out_bytes_sent,
            TF_Status* out_status
        );
        void (*receive_from)(
            TF_Socket* socket,
            void* out_buffer,
            size_t buffer_size,
            size_t* out_bytes_received,
            TF_String* out_sender_host,
            uint16_t* out_sender_port,
            TF_Status* out_status
        );

        // Timeouts, shutdown, and status/error introspection.
        void (*set_send_timeout)(TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status);
        void (*set_receive_timeout)(TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status);

        // how: 0=read, 1=write, 2=both. Half-close, distinct from close_socket.
        void (*shutdown)(TF_Socket* socket, int how, TF_Status* out_status);

        void (*get_status)(TF_Socket* socket, int* out_status);

        // OS/SSL error code behind the last errored status.
        void (*get_error_code)(TF_Socket* socket, int* out_error_code);

        // Introspection.
        void (*get_local_endpoint)(TF_Socket* socket, TF_String* out_host, uint16_t* out_port, TF_Status* out_status);
        void (*get_remote_endpoint)(TF_Socket* socket, TF_String* out_host, uint16_t* out_port, TF_Status* out_status);
        void (*get_protocol)(TF_Socket* socket, TFSocketProtocol* out_protocol);

        // Raw OS handle, for interop/leverager registration.
        void (*get_fd)(TF_Socket* socket, intptr_t* out_fd);

        void (*is_valid)(TF_Socket* socket, int* out_valid);

    } TF_SocketOps;

#define TF_SOCKET_STRUCT_SIZE TF_OFFSET_OF_END(TF_SocketOps, is_valid)

    TF_CAPI_EXPORT void create_socket(TF_SocketOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_socket(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_SOCKET_H_
