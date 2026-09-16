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
    // TF_Client — generic outbound client control. Mirrors interfaces::IClient (on_connect, send(request), create_request (stream_id)) plus the concrete Client's retry(), with connection lifecycle and introspection added to match the other io/ interfaces' depth.
    typedef struct TF_Client
    {
        void* plugin_data;
    } TF_Client;
    typedef void (*TF_Client_ResponseFn)(void* user_data, TF_Response* response, TF_Status* status);
    typedef void (*TF_Client_ConnectFn)(void* user_data, TF_Status* status);
    typedef void (*TF_Client_DisconnectFn)(void* user_data);

    // Plugin-facing vtable registered via create_client.
    typedef struct TF_ClientOps
    {
        size_t struct_size;

        void (*destroy)(TF_Client* client);
        void (*get_name)(TF_Client* client, TF_String* out);

        void (*connect)(TF_Client* client, int64_t timeout_ms, TF_Status* status);
        void (*connect_async)(
            TF_Client* client,
            int64_t timeout_ms,
            TF_Client_ConnectFn completion,
            void* user_data,
            TF_Status* status
        );
        void (*disconnect)(TF_Client* client, TF_Status* status);

        // Disconnect then connect again using the same host/port/config.
        void (*reconnect)(TF_Client* client, TF_Status* status);
        void (*on_disconnect)(TF_Client* client, TF_Client_DisconnectFn handler, void* user_data);
        int (*is_connected)(TF_Client* client);
        void (*get_remote_endpoint)(TF_Client* client, TF_String* out_host, uint16_t* out_port);

        void (*set_keep_alive)(TF_Client* client, int enabled, TF_Status* status);
        int (*is_keep_alive)(TF_Client* client);

        TF_Request* (*create_request)(TF_Client* client, uint32_t stream_id, TF_Status* status);

        void (*send)(TF_Client* client, TF_Request* request, TF_Response* out_response, TF_Status* status);
        void (*send_async)(
            TF_Client* client,
            TF_Request* request,
            TF_Client_ResponseFn completion,
            void* user_data,
            TF_Status* status
        );
        void (*cancel_request)(TF_Client* client, uint32_t stream_id, TF_Status* status);
        void (*list_pending_requests)(TF_Client* client, TF_Vector* out_stream_ids, TF_Status* status);
        size_t (*get_pending_request_count)(TF_Client* client);

        // A lightweight liveness probe distinct from a real request/response round trip (e.g. a transport-level ping frame where the protocol supports one).
        void (*ping)(TF_Client* client, TF_Client_ConnectFn completion, void* user_data, TF_Status* status);

        void (*retry)(TF_Client* client, TF_Status* status);
        void (*set_max_retries)(TF_Client* client, int max_retries, TF_Status* status);
        void (*set_retry_backoff)(TF_Client* client, int64_t backoff_ms, TF_Status* status);
        void (*set_timeout)(TF_Client* client, int64_t timeout_ms, TF_Status* status);

        // Reuses the status-carrying connect-style signature.
        void (*on_error)(TF_Client* client, TF_Client_ConnectFn handler, void* user_data);
        void (*get_last_error)(TF_Client* client, TF_Status* out_status);
        void (*get_stats)(TF_Client* client, TF_Map* out_stats, TF_Status* status);

    } TF_ClientOps;

#define TF_CLIENT_STRUCT_SIZE TF_OFFSET_OF_END(TF_ClientOps, get_stats)

    TF_CAPI_EXPORT void create_client(TF_ClientOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_client(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_CLIENT_H_
