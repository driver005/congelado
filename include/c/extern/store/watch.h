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
    typedef void (*TF_Store_WatchFn)(void* user_data, const TF_String* key, const TF_String* new_value);

    typedef struct TF_Store_Watch
    {
        void* plugin_data;
    } TF_Store_Watch;

    typedef struct TF_Store_WatchOps
    {
        size_t struct_size;
        void (*destroy)(TF_Store_Watch* watch);
        void (*cancel)(TF_Store_Watch* watch);
    } TF_Store_WatchOps;

#define TF_STORE_WATCH_STRUCT_SIZE TF_OFFSET_OF_END(TF_Store_WatchOps, cancel)

    TF_CAPI_EXPORT void create_store_watch(
        TF_Store_WatchOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_store_watch(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_STORE_WATCH_H_
