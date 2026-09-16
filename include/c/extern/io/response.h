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
    // TF_Response — generic protocol-agnostic response message. Mirrors interfaces::io::IResponse's real virtual surface: status + classification, headers, body, cookies, keep-alive, common response metadata getters.
    typedef struct TF_Response
    {
        void* plugin_data;
    } TF_Response;

    // Plugin-facing vtable registered via create_response.
    typedef struct TF_ResponseOps
    {
        size_t struct_size;

        void (*destroy)(TF_Response* response);
        void (*get_name)(TF_Response* response, TF_String* out);

        void (*set_status)(TF_Response* response, int32_t status_code, TF_Status* status);
        int32_t (*get_status)(TF_Response* response);
        void (*get_status_text)(TF_Response* response, TF_String* out);

        void (*set_header)(TF_Response* response, const TF_String* name, const TF_String* value, TF_Status* status);
        void (*add_header)(TF_Response* response, const TF_String* name, const TF_String* value, TF_Status* status);
        void (*remove_header)(TF_Response* response, const TF_String* name, TF_Status* status);
        const TF_String* (*find_header)(TF_Response* response, const TF_String* name);
        void (*get_headers)(TF_Response* response, TF_Map* out_headers, TF_Status* status);

        void (*set_body)(TF_Response* response, const void* data, size_t length, TF_Status* status);
        void (*get_body)(TF_Response* response, const void** out_data, size_t* out_length);

        void (*set_keep_alive)(TF_Response* response, int enabled, TF_Status* status);
        int (*is_keep_alive)(TF_Response* response);

        void (*set_cookie)(
            TF_Response* response,
            const TF_String* name,
            const TF_String* value,
            const TF_Map* attributes,
            TF_Status* status
        );
        void (*get_set_cookies)(TF_Response* response, TF_Vector* out_cookies, TF_Status* status);

        void (*get_content_type)(TF_Response* response, TF_String* out);
        int64_t (*get_content_length)(TF_Response* response);
        void (*get_location)(TF_Response* response, TF_String* out);
        void (*get_etag)(TF_Response* response, TF_String* out);
        void (*get_date)(TF_Response* response, TF_String* out);
        void (*get_server)(TF_Response* response, TF_String* out);
        void (*get_cache_control)(TF_Response* response, TF_String* out);
        void (*get_last_modified)(TF_Response* response, TF_String* out);

        int (*is_informational)(TF_Response* response);
        int (*is_success)(TF_Response* response);
        int (*is_redirection)(TF_Response* response);
        int (*is_client_error)(TF_Response* response);
        int (*is_server_error)(TF_Response* response);

    } TF_ResponseOps;

#define TF_RESPONSE_STRUCT_SIZE TF_OFFSET_OF_END(TF_ResponseOps, is_server_error)

    TF_CAPI_EXPORT void create_response(TF_ResponseOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_response(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_RESPONSE_H_
