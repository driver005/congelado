#ifndef TENSORFLOW_C_EXTERN_SOCKET_H_
#define TENSORFLOW_C_EXTERN_SOCKET_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Socket — generic transport socket, covering TCP/UDP/TLS/QUIC
    // uniformly. Mirrors io::base::socket::Socket<Protocol> directly, with
    // the protocol chosen at construction (a runtime enum here, since C has
    // no template parameter) rather than reinventing a narrower design.
    typedef enum TF_Socket_Protocol
    {
        TF_SOCKET_TCP = 0,
        TF_SOCKET_UDP = 1,
        TF_SOCKET_TLS = 2,
        TF_SOCKET_QUIC = 3
    } TF_Socket_Protocol;

    typedef enum TF_Socket_Status
    {
        TF_SOCKET_ERRORED = 0,
        TF_SOCKET_VALID = 1,
        TF_SOCKET_CLEANLY_DISCONNECTED = 2,
        TF_SOCKET_WOULD_BLOCK = 3,
        TF_SOCKET_TIMED_OUT = 4
    } TF_Socket_Status;

    typedef struct TF_Socket_Handle TF_Socket_Handle;
    typedef void (*TF_Socket_AckFn)(void* user_data, TF_Status* status);
    typedef void (*TF_Socket_AcceptFn)(void* user_data, TF_Socket_Handle* accepted, TF_Status* status);
    typedef void (*TF_Socket_TransferFn)(void* user_data, size_t bytes, TF_Status* status);

    // Plugin-facing vtable registered via init_socket.
    typedef struct TF_Socket
    {
        size_t struct_size;

        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);

        // Client-side: resolves host/port and opens the socket. Server-side:
        // opens the socket that bind/listen will use. protocol is fixed for
        // the handle's lifetime, matching Socket<Protocol>'s compile-time
        // parameter.
        TF_Socket_Handle* (*new_socket)(void* plugin_context, TF_Socket_Protocol protocol, const TF_TString* host, uint16_t port, TF_Status* status);
        void (*close_socket)(void* plugin_context, TF_Socket_Handle* socket);

        // Options.
        void (*set_non_blocking)(void* plugin_context, TF_Socket_Handle* socket, int enabled, TF_Status* status);
        void (*set_reuse_address)(void* plugin_context, TF_Socket_Handle* socket, int enabled, TF_Status* status);
        void (*set_broadcast)(void* plugin_context, TF_Socket_Handle* socket, int enabled, TF_Status* status);

        // No-op for UDP.
        void (*set_tcp_no_delay)(void* plugin_context, TF_Socket_Handle* socket, int enabled, TF_Status* status);

        // TLS (TF_SOCKET_TLS/TF_SOCKET_QUIC only).
        void (*load_certificate)(void* plugin_context, TF_Socket_Handle* socket, const TF_TString* cert_path, const TF_TString* key_path, TF_Status* status);
        void (*generate_certificate)(void* plugin_context, const TF_TString* cert_path, const TF_TString* key_path, TF_Status* status);
        void (*set_verify_peer)(void* plugin_context, TF_Socket_Handle* socket, int enabled, TF_Status* status);

        // Server-side lifecycle.
        void (*bind)(void* plugin_context, TF_Socket_Handle* socket, int allow_unauthorized, TF_Status* status);
        void (*listen)(void* plugin_context, TF_Socket_Handle* socket, int backlog, TF_Status* status);

        // UDP only.
        void (*join_multicast)(void* plugin_context, TF_Socket_Handle* socket, const TF_TString* group, TF_Status* status);

        void (*accept)(void* plugin_context, TF_Socket_Handle* socket, TF_Socket_Handle** out_accepted, TF_Status* status);
        void (*accept_async)(void* plugin_context, TF_Socket_Handle* socket, TF_Socket_AcceptFn completion, void* user_data, TF_Status* status);

        // Client-side lifecycle.
        void (*connect)(void* plugin_context, TF_Socket_Handle* socket, int64_t timeout_ms, TF_Status* status);
        void (*connect_async)(void* plugin_context, TF_Socket_Handle* socket, int64_t timeout_ms, TF_Socket_AckFn completion, void* user_data, TF_Status* status);

        // Data transfer (connected sockets: TCP/TLS/QUIC, or a UDP socket
        // that has itself called connect()).
        void (*send)(void* plugin_context, TF_Socket_Handle* socket, const void* data, size_t length, size_t* out_bytes_sent, TF_Status* status);
        void (*send_async)(
            void* plugin_context,
            TF_Socket_Handle* socket,
            const void* data,
            size_t length,
            TF_Socket_TransferFn completion,
            void* user_data,
            TF_Status* status
        );
        void (*receive)(void* plugin_context, TF_Socket_Handle* socket, void* out_buffer, size_t buffer_size, size_t* out_bytes_received, TF_Status* status);
        void (*receive_async)(
            void* plugin_context,
            TF_Socket_Handle* socket,
            void* out_buffer,
            size_t buffer_size,
            TF_Socket_TransferFn completion,
            void* user_data,
            TF_Status* status
        );

        // Connectionless data transfer (UDP without connect()) — explicit
        // per-datagram destination/sender, matching sendto/recvfrom.
        void (*send_to)(
            void* plugin_context,
            TF_Socket_Handle* socket,
            const void* data,
            size_t length,
            const TF_TString* dest_host,
            uint16_t dest_port,
            size_t* out_bytes_sent,
            TF_Status* status
        );
        void (*receive_from)(
            void* plugin_context,
            TF_Socket_Handle* socket,
            void* out_buffer,
            size_t buffer_size,
            size_t* out_bytes_received,
            TF_String* out_sender_host,
            uint16_t* out_sender_port,
            TF_Status* status
        );

        // Timeouts, shutdown, and status/error introspection.
        void (*set_send_timeout)(void* plugin_context, TF_Socket_Handle* socket, int64_t timeout_ms, TF_Status* status);
        void (*set_receive_timeout)(void* plugin_context, TF_Socket_Handle* socket, int64_t timeout_ms, TF_Status* status);

        // how: 0=read, 1=write, 2=both. Half-close, distinct from
        // close_socket.
        void (*shutdown)(void* plugin_context, TF_Socket_Handle* socket, int how, TF_Status* status);

        // Returns a TF_Socket_Status value.
        int (*get_status)(void* plugin_context, TF_Socket_Handle* socket);

        // OS/SSL error code behind the last errored status.
        int (*get_error_code)(void* plugin_context, TF_Socket_Handle* socket);

        // Introspection.
        void (*get_local_endpoint)(void* plugin_context, TF_Socket_Handle* socket, TF_String* out_host, uint16_t* out_port, TF_Status* status);
        void (*get_remote_endpoint)(void* plugin_context, TF_Socket_Handle* socket, TF_String* out_host, uint16_t* out_port, TF_Status* status);
        TF_Socket_Protocol (*get_protocol)(void* plugin_context, TF_Socket_Handle* socket);

        // Raw OS handle, for interop/leverager registration.
        intptr_t (*get_fd)(void* plugin_context, TF_Socket_Handle* socket);

        int (*is_valid)(void* plugin_context, TF_Socket_Handle* socket);

    } TF_Socket;

#define TF_SOCKET_STRUCT_SIZE TF_OFFSET_OF_END(TF_Socket, is_valid)

    TF_CAPI_EXPORT void init_socket(TF_Socket** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_SOCKET_H_
