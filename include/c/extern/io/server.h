#ifndef TENSORFLOW_C_EXTERN_SERVER_H_
#define TENSORFLOW_C_EXTERN_SERVER_H_

#include "include/c/macros.h"
#include "include/c/extern/io/connection.h"
#include "include/c/extern/io/request.h"
#include "include/c/extern/io/response.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Server — generic server control, absorbing the old TF_Protocol's bind-config (host/port/TLS cert+key) and lifecycle (start/stop/ is_running) with real added depth from the actual Server class (include/io/layer/http2/plugin.cppm): connection tracking, graceful drain, extension registration. Depends on TF_Request/TF_Response for the request-handler callback shape, not on TF_Socket — matches the real design where Server doesn't hold a Socket member itself (composition happens at the concrete plugin's own construction site, not through the abstract interface).
    typedef struct TF_Server
    {
        void* plugin_data;
    } TF_Server;
    typedef void (*TFServerRequestHandler)(
        void* user_data,
        TFServerConnection* connection,
        TF_Request* request,
        TF_Response* response
    );
    typedef void (*TFServerConnectFn)(void* user_data, TFServerConnection* connection);
    typedef void (*TFServerDisconnectFn)(void* user_data, TFServerConnection* connection);

    // Plugin-facing vtable registered via create_server.
    typedef struct TF_ServerOps
    {
        size_t struct_size;

        void (*destroy)(TF_Server* server);
        void (*get_name)(TF_Server* server, TF_String* out_name);

        // Bind configuration.
        void (*get_bind_host)(TF_Server* server, TF_String* out_host);
        void (*get_bind_port)(TF_Server* server, uint16_t* out_port);
        void (*get_tls_cert)(TF_Server* server, TF_String* out_cert);
        void (*get_tls_key)(TF_Server* server, TF_String* out_key);

        void (*set_request_handler)(TF_Server* server, TFServerRequestHandler handler, void* user_data);
        void (*on_connect)(TF_Server* server, TFServerConnectFn handler, void* user_data);
        void (*on_disconnect)(TF_Server* server, TFServerDisconnectFn handler, void* user_data);

        void (*start)(TF_Server* server, TF_Status* out_status);

        // Graceful drain.
        void (*stop)(TF_Server* server, TF_Status* out_status);

        // Stop new connections, keep serving existing ones.
        void (*stop_accepting)(TF_Server* server, TF_Status* out_status);

        // Pairs with stop_accepting.
        void (*resume_accepting)(TF_Server* server, TF_Status* out_status);
        void (*is_running)(TF_Server* server, int* out_running);

        // No active connections.
        void (*is_idle)(TF_Server* server, int* out_idle);

        // Connection limits and lookup.
        void (*set_max_connections)(TF_Server* server, size_t max_connections, TF_Status* out_status);
        void (*get_max_connections)(TF_Server* server, size_t* out_max_connections);
        void (*find_connection)(TF_Server* server, const TF_String* connection_id, TFServerConnection* out_connection, TF_Status* out_status);
        void (*get_connection_id)(TFServerConnection* connection, TF_String* out_connection_id);

        void (*send_response)(TFServerConnection* connection, TF_Response* response, TF_Status* out_status);

        // Send the same response to every currently active connection.
        void (*broadcast)(TF_Server* server, TF_Response* response, TF_Status* out_status);
        void (*close_connection)(TFServerConnection* connection, TF_Status* out_status);
        void (*list_connections)(TF_Server* server, TF_Vector* out_connections, TF_Status* out_status);
        void (*get_connection_count)(TF_Server* server, size_t* out_count);

        // out_stats keys such as total_requests/bytes_sent/bytes_received/ uptime_ms are a documented convention, not enforced by this header.
        void (*get_stats)(TF_Server* server, TF_Map* out_stats, TF_Status* out_status);

        // Extension management (matches the real Server's HttpExtensionRegistry).
        void (*register_extension)(
            TF_Server* server,
            const TF_String* name,
            const TF_Map* config,
            TF_Status* out_status
        );
        void (*unregister_extension)(TF_Server* server, const TF_String* name, TF_Status* out_status);
        void (*list_extensions)(TF_Server* server, TF_Vector* out_names, TF_Status* out_status);

        // Hot cert rotation without a restart.
        void (*reload_certificate)(
            TF_Server* server,
            const TF_String* cert_path,
            const TF_String* key_path,
            TF_Status* out_status
        );

    } TF_ServerOps;

#define TF_SERVER_STRUCT_SIZE TF_OFFSET_OF_END(TF_ServerOps, reload_certificate)

    TF_CAPI_EXPORT void create_server(TF_ServerOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_server(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_SERVER_H_
