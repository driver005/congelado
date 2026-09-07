#ifndef TENSORFLOW_C_EXTERN_STORE_H_
#define TENSORFLOW_C_EXTERN_STORE_H_

#include "c/abi/macros.h"
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
    // TF_Store — generic keyed data store, replacing the separate cache,
    // database, search, and payload domain vtables. Those four are all "persist
    // and retrieve data by key or query" against a different backend
    // (in-memory, SQL, full-text index, blob store) — the same job, not four
    // different ones. Value is an opaque TF_TString blob (JSON or raw bytes,
    // plugin's choice), matching what every source domain already passed.
    //
    // Collections (TF_Store_Handle) are a real concept database/search need
    // that a flat key space can't express — a table, an index, a namespace.
    // Root-level calls with collection == NULL hit an implicit default
    // collection, which is all cache ever needed.
    typedef struct TF_Store_Handle TF_Store_Handle;
    typedef struct TF_Store_Watch TF_Store_Watch;
    typedef struct TF_Store_Transaction TF_Store_Transaction;

    typedef void (*TF_Store_GetCompletionFn)(void* user_data, const TF_TString* value, TF_Status* status);
    typedef void (*TF_Store_MultiGetCompletionFn)(void* user_data, const TF_Map_Handle* results, TF_Status* status);
    typedef void (*TF_Store_SetCompletionFn)(void* user_data, const TF_TString* key, TF_Status* status);
    typedef void (*TF_Store_AckFn)(void* user_data, TF_Status* status);
    typedef void (*TF_Store_ExistsFn)(void* user_data, int exists, TF_Status* status);
    typedef void (*TF_Store_CountFn)(void* user_data, size_t count, TF_Status* status);
    typedef void (*TF_Store_IntFn)(void* user_data, int64_t value, TF_Status* status);
    typedef void (*TF_Store_BoolFn)(void* user_data, int success, TF_Status* status);
    typedef void (*TF_Store_QueryFn)(void* user_data, const TF_Vector_Handle* matches, TF_Status* status);

    // new_value is NULL when the change being reported is a delete.
    typedef void (*TF_Store_WatchFn)(void* user_data, const TF_TString* key, const TF_TString* new_value);

    // Plugin-facing vtable registered via init_store.
    typedef struct TF_Store
    {
        size_t struct_size;

        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        int (*is_connected)(void* plugin_context);

        // Collections/tables/indices.
        TF_Store_Handle* (*open_collection)(void* plugin_context, const TF_TString* name, TF_Status* status);
        void (*close_collection)(void* plugin_context, TF_Store_Handle* collection);
        void (*list_collections)(void* plugin_context, TF_Vector_Handle* out_names, TF_Status* status);
        void (*drop_collection)(void* plugin_context, const TF_TString* name, TF_Status* status);

        // out_stats keys such as item_count/size_bytes are a documented
        // convention, not enforced by this header.
        void (*get_collection_stats)(
            void* plugin_context,
            TF_Store_Handle* collection,
            TF_Map_Handle* out_stats,
            TF_Status* status
        );

        // Core CRUD, single + batch.
        void (*get)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_TString* key,
            TF_Store_GetCompletionFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*multi_get)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_Vector_Handle* keys,
            TF_Store_MultiGetCompletionFn completion,
            void* user_data,
            TF_Status* status
        );

        // key may be NULL to request a plugin-generated key; completion
        // always receives the effective key. ttl_seconds == 0 means no
        // expiry.
        void (*set)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_TString* key,
            const TF_TString* value,
            int64_t ttl_seconds,
            TF_Store_SetCompletionFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*multi_set)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_Map_Handle* entries,
            int64_t ttl_seconds,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*remove)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_TString* key,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*multi_remove)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_Vector_Handle* keys,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*exists)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_TString* key,
            TF_Store_ExistsFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*rename)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_TString* old_key,
            const TF_TString* new_key,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*clear)(void* plugin_context, TF_Store_Handle* collection, TF_Store_AckFn completion, void* user_data, TF_Status* status);

        // Atomic primitives, common to any real KV backend.
        void (*increment)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_TString* key,
            int64_t delta,
            TF_Store_IntFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*compare_and_swap)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_TString* key,
            const TF_TString* expected_value,
            const TF_TString* new_value,
            TF_Store_BoolFn completion,
            void* user_data,
            TF_Status* status
        );

        // TTL lifecycle beyond set-time expiry.
        void (*expire)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_TString* key,
            int64_t ttl_seconds,
            TF_Store_AckFn completion,
            void* user_data,
            TF_Status* status
        );

        void (*get_ttl)(void* plugin_context, TF_Store_Handle* collection, const TF_TString* key, TF_Store_IntFn completion, void* user_data, TF_Status* status);
        void (*persist)(void* plugin_context, TF_Store_Handle* collection, const TF_TString* key, TF_Store_AckFn completion, void* user_data, TF_Status* status);

        // Transactions.
        TF_Store_Transaction* (*begin_transaction)(void* plugin_context, TF_Status* status);
        void (*commit_transaction)(void* plugin_context, TF_Store_Transaction* transaction, TF_Store_AckFn completion, void* user_data, TF_Status* status);
        void (*rollback_transaction)(void* plugin_context, TF_Store_Transaction* transaction);

        // Index management — what search actually is, at the ABI level.
        void (*create_index)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_TString* name,
            const TF_Map_Handle* field_config,
            TF_Status* status
        );
        void (*drop_index)(void* plugin_context, TF_Store_Handle* collection, const TF_TString* name, TF_Status* status);
        void (*list_indexes)(void* plugin_context, TF_Store_Handle* collection, TF_Vector_Handle* out_names, TF_Status* status);

        // Change notification — etcd/redis-keyspace-notification style.
        TF_Store_Watch* (*watch)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_TString* key_prefix,
            TF_Store_WatchFn handler,
            void* user_data,
            TF_Status* status
        );
        void (*unwatch)(void* plugin_context, TF_Store_Watch* watch);

        // filters: arbitrary field->value equality constraints
        // (backend-specific, genuinely open-ended). free_text/sort/offset/
        // limit: universal, explicitly typed — not hidden inside filters.
        // free_text and sort are nullable; limit == 0 means no limit.
        void (*query)(
            void* plugin_context,
            TF_Store_Handle* collection,
            const TF_Map_Handle* filters,
            const TF_TString* free_text,
            const TF_TString* sort,
            size_t offset,
            size_t limit,
            TF_Store_QueryFn completion,
            void* user_data,
            TF_Status* status
        );

        // Operations.
        void (*backup)(void* plugin_context, const TF_TString* destination, TF_Store_AckFn completion, void* user_data, TF_Status* status);
        void (*restore)(void* plugin_context, const TF_TString* source, TF_Store_AckFn completion, void* user_data, TF_Status* status);

    } TF_Store;

#define TF_STORE_STRUCT_SIZE TF_OFFSET_OF_END(TF_Store, restore)

    TF_CAPI_EXPORT void init_store(TF_Store** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_STORE_H_
