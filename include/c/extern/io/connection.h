#ifndef TENSORFLOW_C_EXTERN_IO_CONNECTION_H_
#define TENSORFLOW_C_EXTERN_IO_CONNECTION_H_

#include "include/c/macros.h"
#include "include/c/extern/io/response.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFServerConnection
    {
        void* plugin_data;
    } TFServerConnection;

    typedef struct TFServerConnectionOps
    {
        size_t struct_size;
        void (*create)(TFServerConnection* out_handle);
        void (*destroy)(TFServerConnection* handle);
        void (*get_connection_id)(TFServerConnection* connection, TF_String* out_connection_id);
        void (*send_response)(TFServerConnection* connection, TF_Response* response, TF_Status* out_status);
        void (*close_connection)(TFServerConnection* connection, TF_Status* out_status);
    } TFServerConnectionOps;

#define TF_SERVER_CONNECTION_STRUCT_SIZE TF_OFFSET_OF_END(TFServerConnectionOps, close_connection)

    TF_CAPI_EXPORT void create_server_connection(TFServerConnectionOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_server_connection(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // TENSORFLOW_C_EXTERN_IO_CONNECTION_H_
