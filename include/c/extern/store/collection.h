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

    typedef void (*TFStoreGetCompletionFn)(void* user_data, const TF_String* value, TF_Status* out_status);
    typedef void (*TFStoreMultiGetCompletionFn)(void* user_data, const TF_Map* results, TF_Status* out_status);
    typedef void (*TFStoreSetCompletionFn)(void* user_data, const TF_String* key, TF_Status* out_status);
    typedef void (*TFStoreAckFn)(void* user_data, TF_Status* out_status);
    typedef void (*TFStoreExistsFn)(void* user_data, int exists, TF_Status* out_status);
    typedef void (*TFStoreCountFn)(void* user_data, size_t count, TF_Status* out_status);
    typedef void (*TFStoreIntFn)(void* user_data, int64_t value, TF_Status* out_status);
    typedef void (*TFStoreBoolFn)(void* user_data, int success, TF_Status* out_status);

    typedef struct TFStoreCollection
    {
        void* plugin_data;
    } TFStoreCollection;

    typedef struct TFStoreCollectionOps
    {
        size_t struct_size;
        void (*destroy)(TFStoreCollection* collection);

        // Collection lifecycle.
        void (*close)(TFStoreCollection* collection);
        void (*list)(TFStoreCollection* store, TF_Vector* out_names, TF_Status* out_status);
        void (*drop)(TFStoreCollection* store, const TF_String* name, TF_Status* out_status);
        void (*get_stats)(TFStoreCollection* collection, TF_Map* out_stats, TF_Status* out_status);

        // Core CRUD, single + batch.
        void (*get)(
            TFStoreCollection* collection,
            const TF_String* key,
            TFStoreGetCompletionFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*multi_get)(
            TFStoreCollection* collection,
            const TF_Vector* keys,
            TFStoreMultiGetCompletionFn completion,
            void* user_data,
            TF_Status* out_status
        );
        // key may be NULL to request a plugin-generated key; completion always receives the effective key. ttl_seconds == 0 means no expiry.
        void (*set)(
            TFStoreCollection* collection,
            const TF_String* key,
            const TF_String* value,
            int64_t ttl_seconds,
            TFStoreSetCompletionFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*multi_set)(
            TFStoreCollection* collection,
            const TF_Map* entries,
            int64_t ttl_seconds,
            TFStoreAckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*erase)(
            TFStoreCollection* collection,
            const TF_String* key,
            TFStoreAckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*multi_erase)(
            TFStoreCollection* collection,
            const TF_Vector* keys,
            TFStoreAckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*exists)(
            TFStoreCollection* collection,
            const TF_String* key,
            TFStoreExistsFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*rename)(
            TFStoreCollection* collection,
            const TF_String* old_key,
            const TF_String* new_key,
            TFStoreAckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*clear)(TFStoreCollection* collection, TFStoreAckFn completion, void* user_data, TF_Status* out_status);

        // Atomic primitives.
        void (*increment)(
            TFStoreCollection* collection,
            const TF_String* key,
            int64_t delta,
            TFStoreIntFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*compare_and_swap)(
            TFStoreCollection* collection,
            const TF_String* key,
            const TF_String* expected_value,
            const TF_String* new_value,
            TFStoreBoolFn completion,
            void* user_data,
            TF_Status* out_status
        );

        // TTL lifecycle.
        void (*expire)(
            TFStoreCollection* collection,
            const TF_String* key,
            int64_t ttl_seconds,
            TFStoreAckFn completion,
            void* user_data,
            TF_Status* out_status
        );
        void (*get_ttl)(TFStoreCollection* collection, const TF_String* key, TFStoreIntFn completion, void* user_data, TF_Status* out_status);
        void (*persist)(TFStoreCollection* collection, const TF_String* key, TFStoreAckFn completion, void* user_data, TF_Status* out_status);

    } TFStoreCollectionOps;

#define TF_STORE_COLLECTION_STRUCT_SIZE TF_OFFSET_OF_END(TFStoreCollectionOps, persist)

    TF_CAPI_EXPORT void create_store_collection(
        TFStoreCollectionOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_store_collection(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_STORE_COLLECTION_H_
