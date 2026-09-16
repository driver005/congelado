#ifndef TENSORFLOW_C_EXTERN_SERVER_H_
#define TENSORFLOW_C_EXTERN_SERVER_H_

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
    // TF_Server — generic server control, absorbing the old TF_Protocol's
    // bind-config (host/port/TLS cert+key) and lifecycle (start/stop/
    // is_running) with real added depth from the actual Server class
    // (include/io/layer/http2/plugin.cppm): connection tracking, graceful
    // drain, extension registration. Depends on TF_Request/TF_Response for
    // the request-handler callback shape, not on TF_Socket — matches the
    // real design where Server doesn't hold a Socket member itself
    // (composition happens at the concrete plugin's own construction site,
    // not through the abstract interface).
    typedef struct TF_Server_Handle TF_Server_Handle;
    typedef struct TF_Server_Connection TF_Server_Connection;

    typedef void (*TF_Server_RequestHandler)(
        void* user_data,
        TF_Server_Connection* connection,
        TF_Request_Handle* request,
        TF_Response_Handle* response
    );
    typedef void (*TF_Server_ConnectFn)(void* user_data, TF_Server_Connection* connection);
    typedef void (*TF_Server_DisconnectFn)(void* user_data, TF_Server_Connection* connection);

    // Plugin-facing vtable registered via init_server.
    typedef struct TF_Server
    {
        size_t struct_size;

        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);

        // Bind configuration.
        void (*get_bind_host)(void* plugin_context, TF_String* out);
        uint16_t (*get_bind_port)(void* plugin_context);
        void (*get_tls_cert)(void* plugin_context, TF_String* out);
        void (*get_tls_key)(void* plugin_context, TF_String* out);

        TF_Server_Handle* (*new_server)(void* plugin_context, TF_Status_Handle* status);
        void (*destroy_server)(TF_Server_Handle* server);

        void (*set_request_handler)(TF_Server_Handle* server, TF_Server_RequestHandler handler, void* user_data);
        void (*on_connect)(TF_Server_Handle* server, TF_Server_ConnectFn handler, void* user_data);
        void (*on_disconnect)(TF_Server_Handle* server, TF_Server_DisconnectFn handler, void* user_data);

        void (*start)(TF_Server_Handle* server, TF_Status_Handle* status);

        // Graceful drain.
        void (*stop)(TF_Server_Handle* server, TF_Status_Handle* status);

        // Stop new connections, keep serving existing ones.
        void (*stop_accepting)(TF_Server_Handle* server, TF_Status_Handle* status);

        // Pairs with stop_accepting.
        void (*resume_accepting)(TF_Server_Handle* server, TF_Status_Handle* status);
        int (*is_running)(TF_Server_Handle* server);

        // No active connections.
        int (*is_idle)(TF_Server_Handle* server);

        // Connection limits and lookup.
        void (*set_max_connections)(TF_Server_Handle* server, size_t max_connections, TF_Status_Handle* status);
        size_t (*get_max_connections)(TF_Server_Handle* server);
        TF_Server_Connection* (*find_connection)(TF_Server_Handle* server, const TF_String_Handle* connection_id);
        void (*get_connection_id)(TF_Server_Connection* connection, TF_String* out);

        void (*send_response)(TF_Server_Connection* connection, TF_Response_Handle* response, TF_Status_Handle* status);

        // Send the same response to every currently active connection.
        void (*broadcast)(TF_Server_Handle* server, TF_Response_Handle* response, TF_Status_Handle* status);
        void (*close_connection)(TF_Server_Connection* connection, TF_Status_Handle* status);
        void (*list_connections)(TF_Server_Handle* server, TF_Vector_Handle* out_connections, TF_Status_Handle* status);
        size_t (*get_connection_count)(TF_Server_Handle* server);

        // out_stats keys such as total_requests/bytes_sent/bytes_received/
        // uptime_ms are a documented convention, not enforced by this
        // header.
        void (*get_stats)(TF_Server_Handle* server, TF_Map_Handle* out_stats, TF_Status_Handle* status);

        // Extension management (matches the real Server's
        // HttpExtensionRegistry).
        void (*register_extension)(
            TF_Server_Handle* server,
            const TF_String_Handle* name,
            const TF_Map_Handle* config,
            TF_Status_Handle* status
        );
        void (*unregister_extension)(TF_Server_Handle* server, const TF_String_Handle* name, TF_Status_Handle* status);
        void (*list_extensions)(TF_Server_Handle* server, TF_Vector_Handle* out_names, TF_Status_Handle* status);

        // Hot cert rotation without a restart.
        void (*reload_certificate)(
            TF_Server_Handle* server,
            const TF_String_Handle* cert_path,
            const TF_String_Handle* key_path,
            TF_Status_Handle* status
        );

    } TF_Server;

#define TF_SERVER_STRUCT_SIZE TF_OFFSET_OF_END(TF_Server, reload_certificate)

    TF_CAPI_EXPORT void init_server(TF_Server** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_SERVER_H_
