#ifndef TENSORFLOW_C_EXTERN_REQUEST_H_
#define TENSORFLOW_C_EXTERN_REQUEST_H_

#include "include/c/macros.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

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
        void (*get_name)(TF_Request* request, TF_String* out_name);

        void (*set_method)(TF_Request* request, const TF_String* method, TF_Status* out_status);
        void (*get_method)(TF_Request* request, TF_String* out_method);
        void (*set_path)(TF_Request* request, const TF_String* path, TF_Status* out_status);
        void (*get_path)(TF_Request* request, TF_String* out_path);
        void (*set_scheme)(TF_Request* request, const TF_String* scheme, TF_Status* out_status);
        void (*get_scheme)(TF_Request* request, TF_String* out_scheme);

        // host[:port].
        void (*set_authority)(TF_Request* request, const TF_String* authority, TF_Status* out_status);
        void (*get_authority)(TF_Request* request, TF_String* out_authority);

        void (*set_header)(TF_Request* request, const TF_String* name, const TF_String* value, TF_Status* out_status);

        // Append, for multi-valued headers.
        void (*add_header)(TF_Request* request, const TF_String* name, const TF_String* value, TF_Status* out_status);
        void (*remove_header)(TF_Request* request, const TF_String* name, TF_Status* out_status);

        // Non-owning pointer into the request's own storage; NULL if absent.
        void (*find_header)(TF_Request* request, const TF_String* name, const TF_String** out_value);
        void (*clear_headers)(TF_Request* request, TF_Status* out_status);
        void (*get_headers)(TF_Request* request, TF_Map* out_headers, TF_Status* out_status);

        void (*set_query_param)(TF_Request* request, const TF_String* name, const TF_String* value, TF_Status* out_status);
        void (*get_query_params)(TF_Request* request, TF_Map* out_params, TF_Status* out_status);

        void (*set_body)(TF_Request* request, const void* data, size_t length, TF_Status* out_status);

        // Non-owning; valid until the next mutation.
        void (*get_body)(TF_Request* request, const void** out_data, size_t* out_length);

        void (*set_content_type)(TF_Request* request, const TF_String* content_type, TF_Status* out_status);
        void (*get_content_type)(TF_Request* request, TF_String* out_content_type);
        void (*set_accept)(TF_Request* request, const TF_String* accept, TF_Status* out_status);
        void (*get_accept)(TF_Request* request, TF_String* out_accept);
        void (*set_user_agent)(TF_Request* request, const TF_String* user_agent, TF_Status* out_status);
        void (*get_user_agent)(TF_Request* request, TF_String* out_user_agent);

        void (*set_bearer_auth)(TF_Request* request, const TF_String* token, TF_Status* out_status);
        void (*set_basic_auth)(
            TF_Request* request,
            const TF_String* username,
            const TF_String* password,
            TF_Status* out_status
        );
        void (*get_authorization)(TF_Request* request, TF_String* out_authorization);

        // Peer address, when known.
        void (*set_addr)(TF_Request* request, const TF_String* addr, TF_Status* out_status);
        void (*set_no_decompress)(TF_Request* request, int enabled, TF_Status* out_status);
        void (*set_timeout)(TF_Request* request, int64_t timeout_ms, TF_Status* out_status);
        void (*get_timeout)(TF_Request* request, int64_t* out_timeout_ms);
        void (*set_stream_id)(TF_Request* request, uint32_t stream_id, TF_Status* out_status);
        void (*get_stream_id)(TF_Request* request, uint32_t* out_stream_id);

    } TF_RequestOps;

#define TF_REQUEST_STRUCT_SIZE TF_OFFSET_OF_END(TF_RequestOps, get_stream_id)

    TF_CAPI_EXPORT void create_request(TF_RequestOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_request(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_REQUEST_H_
