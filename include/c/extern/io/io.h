#ifndef TENSORFLOW_C_EXTERN_IO_H_
#define TENSORFLOW_C_EXTERN_IO_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/extern/io/client.h"
#include "c/extern/io/request.h"
#include "c/extern/io/response.h"
#include "c/extern/io/server.h"
#include "c/extern/io/socket.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Io
    {
        void* plugin_data;
        const TF_ClientOps* client_ops;
        const TF_RequestOps* request_ops;
        const TF_ResponseOps* response_ops;
        const TF_ServerOps* server_ops;
        const TF_SocketOps* socket_ops;
    } TF_Io;

    // Real implementation, not declared-only — calls every io/*.h create_x and fills in io's ops fields.
    static inline void init_io(TF_Io* io, TF_Status* out_status)
    {
        TF_ClientOps* client_ops = NULL;
        create_client(&client_ops, &io->plugin_data, out_status);
        io->client_ops = client_ops;

        TF_RequestOps* request_ops = NULL;
        create_request(&request_ops, &io->plugin_data, out_status);
        io->request_ops = request_ops;

        TF_ResponseOps* response_ops = NULL;
        create_response(&response_ops, &io->plugin_data, out_status);
        io->response_ops = response_ops;

        TF_ServerOps* server_ops = NULL;
        create_server(&server_ops, &io->plugin_data, out_status);
        io->server_ops = server_ops;

        TF_SocketOps* socket_ops = NULL;
        create_socket(&socket_ops, &io->plugin_data, out_status);
        io->socket_ops = socket_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_IO_H_
