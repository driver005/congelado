#ifndef TENSORFLOW_C_EXTERN_STORE_ADMIN_H_
#define TENSORFLOW_C_EXTERN_STORE_ADMIN_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Completion callback type
    typedef void (*TFStoreAckFn)(void* user_data, TF_Status* status);

    typedef struct TFStoreAdmin
    {
        void* plugin_data;
    } TFStoreAdmin;

    typedef struct TFStoreAdminOps
    {
        size_t struct_size;

        void (*is_connected)(TFStoreAdmin* manager, int* out_connected);

        // Operations.
        void (*backup)(TFStoreAdmin* manager, const TF_String* destination, TFStoreAckFn completion, void* user_data, TF_Status* out_status);
        void (*restore)(TFStoreAdmin* manager, const TF_String* source, TFStoreAckFn completion, void* user_data, TF_Status* out_status);

    } TFStoreAdminOps;

#define TF_STORE_ADMIN_STRUCT_SIZE TF_OFFSET_OF_END(TFStoreAdminOps, restore)

    TF_CAPI_EXPORT void create_store_admin(TFStoreAdminOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_store_admin(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_STORE_ADMIN_H_
