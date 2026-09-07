#ifndef TENSORFLOW_C_EXTERN_REQUEST_H_
#define TENSORFLOW_C_EXTERN_REQUEST_H_

#include "c/abi/macros.h"
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

        TF_Request_Handle* (*new_request)(void* plugin_context, uint32_t stream_id, TF_Status* status);
        void (*destroy_request)(void* plugin_context, TF_Request_Handle* request);

        void (*set_method)(void* plugin_context, TF_Request_Handle* request, const TF_TString* method, TF_Status* status);
        void (*get_method)(void* plugin_context, TF_Request_Handle* request, TF_String* out);
        void (*set_path)(void* plugin_context, TF_Request_Handle* request, const TF_TString* path, TF_Status* status);
        void (*get_path)(void* plugin_context, TF_Request_Handle* request, TF_String* out);
        void (*set_scheme)(void* plugin_context, TF_Request_Handle* request, const TF_TString* scheme, TF_Status* status);
        void (*get_scheme)(void* plugin_context, TF_Request_Handle* request, TF_String* out);

        // host[:port].
        void (*set_authority)(void* plugin_context, TF_Request_Handle* request, const TF_TString* authority, TF_Status* status);
        void (*get_authority)(void* plugin_context, TF_Request_Handle* request, TF_String* out);

        void (*set_header)(void* plugin_context, TF_Request_Handle* request, const TF_TString* name, const TF_TString* value, TF_Status* status);

        // Append, for multi-valued headers.
        void (*add_header)(void* plugin_context, TF_Request_Handle* request, const TF_TString* name, const TF_TString* value, TF_Status* status);
        void (*remove_header)(void* plugin_context, TF_Request_Handle* request, const TF_TString* name, TF_Status* status);

        // Non-owning pointer into the request's own storage; NULL if absent.
        const TF_TString* (*find_header)(void* plugin_context, TF_Request_Handle* request, const TF_TString* name);
        void (*clear_headers)(void* plugin_context, TF_Request_Handle* request, TF_Status* status);
        void (*get_headers)(void* plugin_context, TF_Request_Handle* request, TF_Map_Handle* out_headers, TF_Status* status);

        void (*set_query_param)(void* plugin_context, TF_Request_Handle* request, const TF_TString* name, const TF_TString* value, TF_Status* status);
        void (*get_query_params)(void* plugin_context, TF_Request_Handle* request, TF_Map_Handle* out_params, TF_Status* status);

        void (*set_body)(void* plugin_context, TF_Request_Handle* request, const void* data, size_t length, TF_Status* status);

        // Non-owning; valid until the next mutation.
        void (*get_body)(void* plugin_context, TF_Request_Handle* request, const void** out_data, size_t* out_length);

        void (*set_content_type)(void* plugin_context, TF_Request_Handle* request, const TF_TString* content_type, TF_Status* status);
        void (*get_content_type)(void* plugin_context, TF_Request_Handle* request, TF_String* out);
        void (*set_accept)(void* plugin_context, TF_Request_Handle* request, const TF_TString* accept, TF_Status* status);
        void (*get_accept)(void* plugin_context, TF_Request_Handle* request, TF_String* out);
        void (*set_user_agent)(void* plugin_context, TF_Request_Handle* request, const TF_TString* user_agent, TF_Status* status);
        void (*get_user_agent)(void* plugin_context, TF_Request_Handle* request, TF_String* out);

        void (*set_bearer_auth)(void* plugin_context, TF_Request_Handle* request, const TF_TString* token, TF_Status* status);
        void (*set_basic_auth)(
            void* plugin_context,
            TF_Request_Handle* request,
            const TF_TString* username,
            const TF_TString* password,
            TF_Status* status
        );
        void (*get_authorization)(void* plugin_context, TF_Request_Handle* request, TF_String* out);

        // Peer address, when known.
        void (*set_addr)(void* plugin_context, TF_Request_Handle* request, const TF_TString* addr, TF_Status* status);
        void (*set_no_decompress)(void* plugin_context, TF_Request_Handle* request, int enabled, TF_Status* status);
        void (*set_timeout)(void* plugin_context, TF_Request_Handle* request, int64_t timeout_ms, TF_Status* status);
        int64_t (*get_timeout)(void* plugin_context, TF_Request_Handle* request);
        void (*set_stream_id)(void* plugin_context, TF_Request_Handle* request, uint32_t stream_id, TF_Status* status);
        uint32_t (*get_stream_id)(void* plugin_context, TF_Request_Handle* request);

    } TF_Request;

#define TF_REQUEST_STRUCT_SIZE TF_OFFSET_OF_END(TF_Request, get_stream_id)

    TF_CAPI_EXPORT void init_request(TF_Request** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_REQUEST_H_
