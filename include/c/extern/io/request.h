#ifndef TENSORFLOW_C_EXTERN_REQUEST_H_
#define TENSORFLOW_C_EXTERN_REQUEST_H_

#include "c/macros.h"
#include "c/intern/tf_map.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Request — generic protocol-agnostic request message. Mirrors
    // interfaces::io::IRequest's real virtual surface (method/path/scheme/
    // authority, multi-value headers, query params, body, content-type/
    // accept/user-agent, bearer/basic auth, timeout, stream_id,
    // no-decompress). Deliberately no HTTP-only method enum — method is
    // just a string, so a GraphQL/gRPC/raw-protocol request isn't forced
    // into GET/POST.
    typedef struct TF_Request_Handle TF_Request_Handle;

    // Plugin-facing vtable registered via init_request.
    typedef struct TF_Request
    {
        size_t struct_size;

        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);

        TF_Request_Handle* (*new_request)(void* plugin_context, uint32_t stream_id, TF_Status_Handle* status);
        void (*destroy_request)(TF_Request_Handle* request);

        void (*set_method)(TF_Request_Handle* request, const TF_String_Handle* method, TF_Status_Handle* status);
        void (*get_method)(TF_Request_Handle* request, TF_String* out);
        void (*set_path)(TF_Request_Handle* request, const TF_String_Handle* path, TF_Status_Handle* status);
        void (*get_path)(TF_Request_Handle* request, TF_String* out);
        void (*set_scheme)(TF_Request_Handle* request, const TF_String_Handle* scheme, TF_Status_Handle* status);
        void (*get_scheme)(TF_Request_Handle* request, TF_String* out);

        // host[:port].
        void (*set_authority)(TF_Request_Handle* request, const TF_String_Handle* authority, TF_Status_Handle* status);
        void (*get_authority)(TF_Request_Handle* request, TF_String* out);

        void (*set_header)(TF_Request_Handle* request, const TF_String_Handle* name, const TF_String_Handle* value, TF_Status_Handle* status);

        // Append, for multi-valued headers.
        void (*add_header)(TF_Request_Handle* request, const TF_String_Handle* name, const TF_String_Handle* value, TF_Status_Handle* status);
        void (*remove_header)(TF_Request_Handle* request, const TF_String_Handle* name, TF_Status_Handle* status);

        // Non-owning pointer into the request's own storage; NULL if absent.
        const TF_String_Handle* (*find_header)(TF_Request_Handle* request, const TF_String_Handle* name);
        void (*clear_headers)(TF_Request_Handle* request, TF_Status_Handle* status);
        void (*get_headers)(TF_Request_Handle* request, TF_Map_Handle* out_headers, TF_Status_Handle* status);

        void (*set_query_param)(TF_Request_Handle* request, const TF_String_Handle* name, const TF_String_Handle* value, TF_Status_Handle* status);
        void (*get_query_params)(TF_Request_Handle* request, TF_Map_Handle* out_params, TF_Status_Handle* status);

        void (*set_body)(TF_Request_Handle* request, const void* data, size_t length, TF_Status_Handle* status);

        // Non-owning; valid until the next mutation.
        void (*get_body)(TF_Request_Handle* request, const void** out_data, size_t* out_length);

        void (*set_content_type)(TF_Request_Handle* request, const TF_String_Handle* content_type, TF_Status_Handle* status);
        void (*get_content_type)(TF_Request_Handle* request, TF_String* out);
        void (*set_accept)(TF_Request_Handle* request, const TF_String_Handle* accept, TF_Status_Handle* status);
        void (*get_accept)(TF_Request_Handle* request, TF_String* out);
        void (*set_user_agent)(TF_Request_Handle* request, const TF_String_Handle* user_agent, TF_Status_Handle* status);
        void (*get_user_agent)(TF_Request_Handle* request, TF_String* out);

        void (*set_bearer_auth)(TF_Request_Handle* request, const TF_String_Handle* token, TF_Status_Handle* status);
        void (*set_basic_auth)(
            TF_Request_Handle* request,
            const TF_String_Handle* username,
            const TF_String_Handle* password,
            TF_Status_Handle* status
        );
        void (*get_authorization)(TF_Request_Handle* request, TF_String* out);

        // Peer address, when known.
        void (*set_addr)(TF_Request_Handle* request, const TF_String_Handle* addr, TF_Status_Handle* status);
        void (*set_no_decompress)(TF_Request_Handle* request, int enabled, TF_Status_Handle* status);
        void (*set_timeout)(TF_Request_Handle* request, int64_t timeout_ms, TF_Status_Handle* status);
        int64_t (*get_timeout)(TF_Request_Handle* request);
        void (*set_stream_id)(TF_Request_Handle* request, uint32_t stream_id, TF_Status_Handle* status);
        uint32_t (*get_stream_id)(TF_Request_Handle* request);

    } TF_Request;

#define TF_REQUEST_STRUCT_SIZE TF_OFFSET_OF_END(TF_Request, get_stream_id)

    TF_CAPI_EXPORT void init_request(TF_Request** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_REQUEST_H_
