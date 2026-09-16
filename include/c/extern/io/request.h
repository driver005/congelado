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
    // TF_Request — generic protocol-agnostic request message. Mirrors interfaces::io::IRequest's real virtual surface (method/path/scheme/ authority, multi-value headers, query params, body, content-type/ accept/user-agent, bearer/basic auth, timeout, stream_id, no-decompress). Deliberately no HTTP-only method enum — method is just a string, so a GraphQL/gRPC/raw-protocol request isn't forced into GET/POST.
    typedef struct TF_Request
    {
        void* plugin_data;
    } TF_Request;

    // Plugin-facing vtable registered via create_request.
    typedef struct TF_RequestOps
    {
        size_t struct_size;

        void (*destroy)(TF_Request* request);
        void (*get_name)(TF_Request* request, TF_String* out);

        void (*set_method)(TF_Request* request, const TF_String* method, TF_Status* status);
        void (*get_method)(TF_Request* request, TF_String* out);
        void (*set_path)(TF_Request* request, const TF_String* path, TF_Status* status);
        void (*get_path)(TF_Request* request, TF_String* out);
        void (*set_scheme)(TF_Request* request, const TF_String* scheme, TF_Status* status);
        void (*get_scheme)(TF_Request* request, TF_String* out);

        // host[:port].
        void (*set_authority)(TF_Request* request, const TF_String* authority, TF_Status* status);
        void (*get_authority)(TF_Request* request, TF_String* out);

        void (*set_header)(TF_Request* request, const TF_String* name, const TF_String* value, TF_Status* status);

        // Append, for multi-valued headers.
        void (*add_header)(TF_Request* request, const TF_String* name, const TF_String* value, TF_Status* status);
        void (*remove_header)(TF_Request* request, const TF_String* name, TF_Status* status);

        // Non-owning pointer into the request's own storage; NULL if absent.
        const TF_String* (*find_header)(TF_Request* request, const TF_String* name);
        void (*clear_headers)(TF_Request* request, TF_Status* status);
        void (*get_headers)(TF_Request* request, TF_Map* out_headers, TF_Status* status);

        void (*set_query_param)(TF_Request* request, const TF_String* name, const TF_String* value, TF_Status* status);
        void (*get_query_params)(TF_Request* request, TF_Map* out_params, TF_Status* status);

        void (*set_body)(TF_Request* request, const void* data, size_t length, TF_Status* status);

        // Non-owning; valid until the next mutation.
        void (*get_body)(TF_Request* request, const void** out_data, size_t* out_length);

        void (*set_content_type)(TF_Request* request, const TF_String* content_type, TF_Status* status);
        void (*get_content_type)(TF_Request* request, TF_String* out);
        void (*set_accept)(TF_Request* request, const TF_String* accept, TF_Status* status);
        void (*get_accept)(TF_Request* request, TF_String* out);
        void (*set_user_agent)(TF_Request* request, const TF_String* user_agent, TF_Status* status);
        void (*get_user_agent)(TF_Request* request, TF_String* out);

        void (*set_bearer_auth)(TF_Request* request, const TF_String* token, TF_Status* status);
        void (*set_basic_auth)(
            TF_Request* request,
            const TF_String* username,
            const TF_String* password,
            TF_Status* status
        );
        void (*get_authorization)(TF_Request* request, TF_String* out);

        // Peer address, when known.
        void (*set_addr)(TF_Request* request, const TF_String* addr, TF_Status* status);
        void (*set_no_decompress)(TF_Request* request, int enabled, TF_Status* status);
        void (*set_timeout)(TF_Request* request, int64_t timeout_ms, TF_Status* status);
        int64_t (*get_timeout)(TF_Request* request);
        void (*set_stream_id)(TF_Request* request, uint32_t stream_id, TF_Status* status);
        uint32_t (*get_stream_id)(TF_Request* request);

    } TF_RequestOps;

#define TF_REQUEST_STRUCT_SIZE TF_OFFSET_OF_END(TF_RequestOps, get_stream_id)

    TF_CAPI_EXPORT void create_request(TF_RequestOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_request(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_REQUEST_H_
