#ifndef CONGELADO_C_STORE_COLLECTION_H_
#define CONGELADO_C_STORE_COLLECTION_H_

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

    typedef void (*TF_Store_GetCompletionFn)(void* user_data, const TF_String* value, TF_Status* out_status);
    typedef void (*TF_Store_MultiGetCompletionFn)(void* user_data, const TF_Map* results, TF_Status* out_status);
    typedef void (*TF_Store_SetCompletionFn)(void* user_data, const TF_String* key, TF_Status* out_status);
    typedef void (*TF_Store_AckFn)(void* user_data, TF_Status* out_status);
    typedef void (*TF_Store_ExistsFn)(void* user_data, int exists, TF_Status* out_status);
    typedef void (*TF_Store_CountFn)(void* user_data, size_t count, TF_Status* out_status);
    typedef void (*TF_Store_IntFn)(void* user_data, int64_t value, TF_Status* out_status);
    typedef void (*TF_Store_BoolFn)(void* user_data, int success, TF_Status* out_status);

    typedef struct TF_Store_Collection
    {
        void* plugin_data;
    } TF_Store_Collection;

    typedef struct TF_Store_CollectionOps
    {
        size_t struct_size;
        void (*destroy)(TF_Store_Collection* collection);

        // Collection lifecycle.
        void (*close)(TF_Store_Collection* collection);
        void (*list)(TF_Store_Collection* store, TF_Vector* out_names, TF_Status* out_status);
        void (*drop)(TF_Store_Collection* store, const TF_String* name, TF_Status* out_status);
        void (*get_stats)(TF_Store_Collection* collection, TF_Map* out_stats, TF_Status* out_status);

        // Core CRUD, single + batch.
        void (*get)(
            TF_Store_Collection* collection,
            const TF_String* key,
            TF_Store_GetCompletionFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*multi_get)(
            TF_Store_Collection* collection,
            const TF_Vector* keys,
            TF_Store_MultiGetCompletionFn completion,
            void* user_data,
            TF_Status* out_status
        );
        // key may be NULL to request a plugin-generated key; completion always receives the effective key. ttl_seconds == 0 means no expiry.
        void (*set)(
            TF_Store_Collection* collection,
            const TF_String* key,
            const TF_String* value,
            int64_t ttl_seconds,
            TF_Store_SetCompletionFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*multi_set)(
            TF_Store_Collection* collection,
            const TF_Map* entries,
            int64_t ttl_seconds,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*remove)(
            TF_Store_Collection* collection,
            const TF_String* key,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*multi_remove)(
            TF_Store_Collection* collection,
            const TF_Vector* keys,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*exists)(
            TF_Store_Collection* collection,
            const TF_String* key,
            TF_Store_ExistsFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*rename)(
            TF_Store_Collection* collection,
            const TF_String* old_key,
            const TF_String* new_key,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*clear)(TF_Store_Collection* collection, TF_Store_AckFn completion, void* user_data, TF_Status* out_status);

        // Atomic primitives.
        void (*increment)(
            TF_Store_Collection* collection,
            const TF_String* key,
            int64_t delta,
            TF_Store_IntFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*compare_and_swap)(
            TF_Store_Collection* collection,
            const TF_String* key,
            const TF_String* expected_value,
            const TF_String* new_value,
            TF_Store_BoolFn completion,
            void* user_data,
            TF_Status* out_status
        );

        // TTL lifecycle.
        void (*expire)(
            TF_Store_Collection* collection,
            const TF_String* key,
            int64_t ttl_seconds,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*get_ttl)(TF_Store_Collection* collection, const TF_String* key, TF_Store_IntFn completion, void* user_data, TF_Status* out_status);
        void (*persist)(TF_Store_Collection* collection, const TF_String* key, TF_Store_AckFn completion, void* user_data, TF_Status* out_status);

    } TF_Store_CollectionOps;

#define TF_STORE_COLLECTION_STRUCT_SIZE TF_OFFSET_OF_END(TF_Store_CollectionOps, persist)

    TF_CAPI_EXPORT void create_store_collection(
        TF_Store_CollectionOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_store_collection(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_STORE_COLLECTION_H_
