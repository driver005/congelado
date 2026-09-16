#ifndef TENSORFLOW_C_EXTERN_CLIENT_H_
#define TENSORFLOW_C_EXTERN_CLIENT_H_

#include "c/macros.h"
#include "c/extern/io/request.h"
#include "c/extern/io/response.h"
#include "c/intern/tf_map.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_vector.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Client — generic outbound client control. Mirrors
    // interfaces::IClient (on_connect, send(request), create_request
    // (stream_id)) plus the concrete Client's retry(), with connection
    // lifecycle and introspection added to match the other io/ interfaces'
    // depth.
    typedef struct TF_Client_Handle TF_Client_Handle;
    typedef void (*TF_Client_ResponseFn)(void* user_data, TF_Response_Handle* response, TF_Status_Handle* status);
    typedef void (*TF_Client_ConnectFn)(void* user_data, TF_Status_Handle* status);
    typedef void (*TF_Client_DisconnectFn)(void* user_data);

    // Plugin-facing vtable registered via init_client.
    typedef struct TF_Client
    {
        size_t struct_size;

        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);

        TF_Client_Handle* (*new_client)(void* plugin_context, const TF_String_Handle* host, uint16_t port, TF_Status_Handle* status);
        void (*destroy_client)(TF_Client_Handle* client);

        void (*connect)(TF_Client_Handle* client, int64_t timeout_ms, TF_Status_Handle* status);
        void (*connect_async)(
            TF_Client_Handle* client,
            int64_t timeout_ms,
            TF_Client_ConnectFn completion,
            void* user_data,
            TF_Status_Handle* status
        );
        void (*disconnect)(TF_Client_Handle* client, TF_Status_Handle* status);

        // Disconnect then connect again using the same host/port/config.
        void (*reconnect)(TF_Client_Handle* client, TF_Status_Handle* status);
        void (*on_disconnect)(TF_Client_Handle* client, TF_Client_DisconnectFn handler, void* user_data);
        int (*is_connected)(TF_Client_Handle* client);
        void (*get_remote_endpoint)(TF_Client_Handle* client, TF_String* out_host, uint16_t* out_port);

        void (*set_keep_alive)(TF_Client_Handle* client, int enabled, TF_Status_Handle* status);
        int (*is_keep_alive)(TF_Client_Handle* client);

        TF_Request_Handle* (*create_request)(TF_Client_Handle* client, uint32_t stream_id, TF_Status_Handle* status);

        void (*send)(TF_Client_Handle* client, TF_Request_Handle* request, TF_Response_Handle* out_response, TF_Status_Handle* status);
        void (*send_async)(
            TF_Client_Handle* client,
            TF_Request_Handle* request,
            TF_Client_ResponseFn completion,
            void* user_data,
            TF_Status_Handle* status
        );
        void (*cancel_request)(TF_Client_Handle* client, uint32_t stream_id, TF_Status_Handle* status);
        void (*list_pending_requests)(TF_Client_Handle* client, TF_Vector_Handle* out_stream_ids, TF_Status_Handle* status);
        size_t (*get_pending_request_count)(TF_Client_Handle* client);

        // A lightweight liveness probe distinct from a real request/response
        // round trip (e.g. a transport-level ping frame where the protocol
        // supports one).
        void (*ping)(TF_Client_Handle* client, TF_Client_ConnectFn completion, void* user_data, TF_Status_Handle* status);

        void (*retry)(TF_Client_Handle* client, TF_Status_Handle* status);
        void (*set_max_retries)(TF_Client_Handle* client, int max_retries, TF_Status_Handle* status);
        void (*set_retry_backoff)(TF_Client_Handle* client, int64_t backoff_ms, TF_Status_Handle* status);
        void (*set_timeout)(TF_Client_Handle* client, int64_t timeout_ms, TF_Status_Handle* status);

        // Reuses the status-carrying connect-style signature.
        void (*on_error)(TF_Client_Handle* client, TF_Client_ConnectFn handler, void* user_data);
        void (*get_last_error)(TF_Client_Handle* client, TF_Status_Handle* out_status);
        void (*get_stats)(TF_Client_Handle* client, TF_Map_Handle* out_stats, TF_Status_Handle* status);

    } TF_Client;

#define TF_CLIENT_STRUCT_SIZE TF_OFFSET_OF_END(TF_Client, get_stats)

    TF_CAPI_EXPORT void init_client(TF_Client** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_CLIENT_H_
