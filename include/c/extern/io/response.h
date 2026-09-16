#ifndef TENSORFLOW_C_EXTERN_RESPONSE_H_
#define TENSORFLOW_C_EXTERN_RESPONSE_H_

#include "c/macros.h"
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
    // TF_Response — generic protocol-agnostic response message. Mirrors
    // interfaces::io::IResponse's real virtual surface: status +
    // classification, headers, body, cookies, keep-alive, common response
    // metadata getters.
    typedef struct TF_Response_Handle TF_Response_Handle;

    // Plugin-facing vtable registered via init_response.
    typedef struct TF_Response
    {
        size_t struct_size;

        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);

        TF_Response_Handle* (*new_response)(void* plugin_context, uint32_t stream_id, TF_Status_Handle* status);
        void (*destroy_response)(TF_Response_Handle* response);

        void (*set_status)(TF_Response_Handle* response, int32_t status_code, TF_Status_Handle* status);
        int32_t (*get_status)(TF_Response_Handle* response);
        void (*get_status_text)(TF_Response_Handle* response, TF_String* out);

        void (*set_header)(TF_Response_Handle* response, const TF_String_Handle* name, const TF_String_Handle* value, TF_Status_Handle* status);
        void (*add_header)(TF_Response_Handle* response, const TF_String_Handle* name, const TF_String_Handle* value, TF_Status_Handle* status);
        void (*remove_header)(TF_Response_Handle* response, const TF_String_Handle* name, TF_Status_Handle* status);
        const TF_String_Handle* (*find_header)(TF_Response_Handle* response, const TF_String_Handle* name);
        void (*get_headers)(TF_Response_Handle* response, TF_Map_Handle* out_headers, TF_Status_Handle* status);

        void (*set_body)(TF_Response_Handle* response, const void* data, size_t length, TF_Status_Handle* status);
        void (*get_body)(TF_Response_Handle* response, const void** out_data, size_t* out_length);

        void (*set_keep_alive)(TF_Response_Handle* response, int enabled, TF_Status_Handle* status);
        int (*is_keep_alive)(TF_Response_Handle* response);

        void (*set_cookie)(
            TF_Response_Handle* response,
            const TF_String_Handle* name,
            const TF_String_Handle* value,
            const TF_Map_Handle* attributes,
            TF_Status_Handle* status
        );
        void (*get_set_cookies)(TF_Response_Handle* response, TF_Vector_Handle* out_cookies, TF_Status_Handle* status);

        void (*get_content_type)(TF_Response_Handle* response, TF_String* out);
        int64_t (*get_content_length)(TF_Response_Handle* response);
        void (*get_location)(TF_Response_Handle* response, TF_String* out);
        void (*get_etag)(TF_Response_Handle* response, TF_String* out);
        void (*get_date)(TF_Response_Handle* response, TF_String* out);
        void (*get_server)(TF_Response_Handle* response, TF_String* out);
        void (*get_cache_control)(TF_Response_Handle* response, TF_String* out);
        void (*get_last_modified)(TF_Response_Handle* response, TF_String* out);

        int (*is_informational)(TF_Response_Handle* response);
        int (*is_success)(TF_Response_Handle* response);
        int (*is_redirection)(TF_Response_Handle* response);
        int (*is_client_error)(TF_Response_Handle* response);
        int (*is_server_error)(TF_Response_Handle* response);

    } TF_Response;

#define TF_RESPONSE_STRUCT_SIZE TF_OFFSET_OF_END(TF_Response, is_server_error)

    TF_CAPI_EXPORT void init_response(TF_Response** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_RESPONSE_H_
