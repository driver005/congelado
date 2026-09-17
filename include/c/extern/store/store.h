#ifndef TENSORFLOW_C_EXTERN_STORE_H_
#define TENSORFLOW_C_EXTERN_STORE_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include "c/extern/store/collection.h"
#include "c/extern/store/transaction.h"
#include "c/extern/store/watch.h"
#include "c/extern/store/index.h"
#include "c/extern/store/query.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Store
    {
        void* plugin_data;
        const TF_Store_CollectionOps* collection_ops;
        const TF_Store_TransactionOps* transaction_ops;
        const TF_Store_WatchOps* watch_ops;
        const TF_Store_IndexOps* index_ops;
        const TF_Store_QueryOps* query_ops;
    } TF_Store;

    typedef struct TF_StoreOps
    {
        size_t struct_size;

        void (*destroy)(TF_Store* store);
        void (*get_name)(TF_Store* store, TF_String* out_name);
        void (*is_connected)(TF_Store* store, int* out_connected);

        // Operations.
        void (*backup)(TF_Store* store, const TF_String* destination, TF_Store_AckFn completion, void* user_data, TF_Status* out_status);
        void (*restore)(TF_Store* store, const TF_String* source, TF_Store_AckFn completion, void* user_data, TF_Status* out_status);

    } TF_StoreOps;

#define TF_STORE_STRUCT_SIZE TF_OFFSET_OF_END(TF_StoreOps, restore)

    TF_CAPI_EXPORT void create_store(TF_StoreOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_store(void* plugin_context);

    static inline void init_store(TF_StoreOps** ops, TF_Store* store, TF_Status* out_status)
    {
        create_store(ops, &store->plugin_data, out_status);

        TF_Store_CollectionOps* collection_ops = NULL;
        create_store_collection(&collection_ops, &store->plugin_data, out_status);
        store->collection_ops = collection_ops;

        TF_Store_TransactionOps* transaction_ops = NULL;
        create_store_transaction(&transaction_ops, &store->plugin_data, out_status);
        store->transaction_ops = transaction_ops;

        TF_Store_WatchOps* watch_ops = NULL;
        create_store_watch(&watch_ops, &store->plugin_data, out_status);
        store->watch_ops = watch_ops;

        TF_Store_IndexOps* index_ops = NULL;
        create_store_index(&index_ops, &store->plugin_data, out_status);
        store->index_ops = index_ops;

        TF_Store_QueryOps* query_ops = NULL;
        create_store_query(&query_ops, &store->plugin_data, out_status);
        store->query_ops = query_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_STORE_H_
