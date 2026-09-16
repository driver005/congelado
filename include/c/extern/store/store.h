#ifndef TENSORFLOW_C_EXTERN_STORE_H_
#define TENSORFLOW_C_EXTERN_STORE_H_

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
    // TF_Store — generic keyed data store, replacing the separate cache, database, search, and payload domain vtables. Those four are all "persist and retrieve data by key or query" against a different backend (in-memory, SQL, full-text index, blob store) — the same job, not four different ones. Value is an opaque TF_String blob (JSON or raw bytes, plugin's choice), matching what every source domain already passed.
    //
    // Collections (TF_Store) are a real concept database/search need that a flat key space can't express — a table, an index, a namespace. Root-level calls with collection == NULL hit an implicit default collection, which is all cache ever needed.
    typedef struct TF_Store
    {
        void* plugin_data;
    } TF_Store;
    typedef struct TF_Store_Watch TF_Store_Watch;
    typedef struct TF_Store_Transaction TF_Store_Transaction;

    typedef void (*TF_Store_GetCompletionFn)(void* user_data, const TF_String* value, TF_Status* status);
    typedef void (*TF_Store_MultiGetCompletionFn)(void* user_data, const TF_Map* results, TF_Status* status);
    typedef void (*TF_Store_SetCompletionFn)(void* user_data, const TF_String* key, TF_Status* status);
    typedef void (*TF_Store_AckFn)(void* user_data, TF_Status* status);
    typedef void (*TF_Store_ExistsFn)(void* user_data, int exists, TF_Status* status);
    typedef void (*TF_Store_CountFn)(void* user_data, size_t count, TF_Status* status);
    typedef void (*TF_Store_IntFn)(void* user_data, int64_t value, TF_Status* status);
    typedef void (*TF_Store_BoolFn)(void* user_data, int success, TF_Status* status);
    typedef void (*TF_Store_QueryFn)(void* user_data, const TF_Vector* matches, TF_Status* status);

    // new_value is NULL when the change being reported is a delete.
    typedef void (*TF_Store_WatchFn)(void* user_data, const TF_String* key, const TF_String* new_value);

    // Plugin-facing vtable registered via create_store.
    typedef struct TF_StoreOps
    {
        size_t struct_size;

        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        int (*is_connected)(void* plugin_context);

        // Collections/tables/indices.
        TF_Store* (*open_collection)(void* plugin_context, const TF_String* name, TF_Status* status);
        void (*close_collection)(TF_Store* collection);
        void (*list_collections)(void* plugin_context, TF_Vector* out_names, TF_Status* status);
        void (*drop_collection)(void* plugin_context, const TF_String* name, TF_Status* status);

        // out_stats keys such as item_count/size_bytes are a documented convention, not enforced by this header.
        void (*get_collection_stats)(
            TF_Store* collection,
            TF_Map* out_stats,
            TF_Status* status
        );

        // Core CRUD, single + batch.
        void (*get)(
            TF_Store* collection,
            const TF_String* key,
            TF_Store_GetCompletionFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*multi_get)(
            TF_Store* collection,
            const TF_Vector* keys,
            TF_Store_MultiGetCompletionFn completion,
            void* user_data,
            TF_Status* status
        );

        // key may be NULL to request a plugin-generated key; completion always receives the effective key. ttl_seconds == 0 means no expiry.
        void (*set)(
            TF_Store* collection,
            const TF_String* key,
            const TF_String* value,
            int64_t ttl_seconds,
            TF_Store_SetCompletionFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*multi_set)(
            TF_Store* collection,
            const TF_Map* entries,
            int64_t ttl_seconds,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*remove)(
            TF_Store* collection,
            const TF_String* key,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*multi_remove)(
            TF_Store* collection,
            const TF_Vector* keys,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*exists)(
            TF_Store* collection,
            const TF_String* key,
            TF_Store_ExistsFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*rename)(
            TF_Store* collection,
            const TF_String* old_key,
            const TF_String* new_key,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*clear)(TF_Store* collection, TF_Store_AckFn completion, void* user_data, TF_Status* status);

        // Atomic primitives, common to any real KV backend.
        void (*increment)(
            TF_Store* collection,
            const TF_String* key,
            int64_t delta,
            TF_Store_IntFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*compare_and_swap)(
            TF_Store* collection,
            const TF_String* key,
            const TF_String* expected_value,
            const TF_String* new_value,
            TF_Store_BoolFn completion,
            void* user_data,
            TF_Status* status
        );

        // TTL lifecycle beyond set-time expiry.
        void (*expire)(
            TF_Store* collection,
            const TF_String* key,
            int64_t ttl_seconds,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*get_ttl)(TF_Store* collection, const TF_String* key, TF_Store_IntFn completion, void* user_data, TF_Status* status);
        void (*persist)(TF_Store* collection, const TF_String* key, TF_Store_AckFn completion, void* user_data, TF_Status* status);

        // Transactions.
        TF_Store_Transaction* (*begin_transaction)(void* plugin_context, TF_Status* status);
        void (*commit_transaction)(TF_Store_Transaction* transaction, TF_Store_AckFn completion, void* user_data, TF_Status* status);
        void (*rollback_transaction)(TF_Store_Transaction* transaction);

        // Index management — what search actually is, at the ABI level.
        void (*create_index)(
            TF_Store* collection,
            const TF_String* name,
            const TF_Map* field_config,
            TF_Status* status
        );
        void (*drop_index)(TF_Store* collection, const TF_String* name, TF_Status* status);
        void (*list_indexes)(TF_Store* collection, TF_Vector* out_names, TF_Status* status);

        // Change notification — etcd/redis-keyspace-notification style.
        TF_Store_Watch* (*watch)(
            TF_Store* collection,
            const TF_String* key_prefix,
            TF_Store_WatchFn handler,
            void* user_data,
            TF_Status* status
        );
        void (*unwatch)(TF_Store_Watch* watch);

        // filters: arbitrary field->value equality constraints (backend-specific, genuinely open-ended). free_text/sort/offset/ limit: universal, explicitly typed — not hidden inside filters. free_text and sort are nullable; limit == 0 means no limit.
        void (*query)(
            TF_Store* collection,
            const TF_Map* filters,
            const TF_String* free_text,
            const TF_String* sort,
            size_t offset,
            size_t limit,
            TF_Store_QueryFn completion,
            void* user_data,
            TF_Status* status
        );

        // Operations.
        void (*backup)(void* plugin_context, const TF_String* destination, TF_Store_AckFn completion, void* user_data, TF_Status* status);
        void (*restore)(void* plugin_context, const TF_String* source, TF_Store_AckFn completion, void* user_data, TF_Status* status);

    } TF_StoreOps;

#define TF_STORE_STRUCT_SIZE TF_OFFSET_OF_END(TF_StoreOps, restore)

    TF_CAPI_EXPORT void create_store(TF_StoreOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_store(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_STORE_H_
