#ifndef CONGELADO_C_STORE_WATCH_H_
#define CONGELADO_C_STORE_WATCH_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // new_value is NULL when the change being reported is a delete.
    typedef void (*TFStoreWatchFn)(void* user_data, const TF_String* key, const TF_String* new_value);

    typedef struct TFStoreWatch
    {
        void* plugin_data;
    } TFStoreWatch;

    typedef struct TFStoreWatchOps
    {
        size_t struct_size;
        void (*destroy)(TFStoreWatch* watch);
        void (*cancel)(TFStoreWatch* watch);
    } TFStoreWatchOps;

#define TF_STORE_WATCH_STRUCT_SIZE TF_OFFSET_OF_END(TFStoreWatchOps, cancel)

    TF_CAPI_EXPORT void create_store_watch(
        TFStoreWatchOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_store_watch(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_STORE_WATCH_H_
