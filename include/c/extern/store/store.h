#ifndef TENSORFLOW_C_EXTERN_STORE_H_
#define TENSORFLOW_C_EXTERN_STORE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include "include/c/extern/store/collection.h"
#include "include/c/extern/store/transaction.h"
#include "include/c/extern/store/watch.h"
#include "include/c/extern/store/index.h"
#include "include/c/extern/store/query.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Store
    {
        void* plugin_data;
        const TFStoreCollectionOps* collection_ops;
        const TFStoreTransactionOps* transaction_ops;
        const TFStoreWatchOps* watch_ops;
        const TFStoreIndexOps* index_ops;
        const TFStoreQueryOps* query_ops;
    } TF_Store;

    typedef struct TF_StoreOps
    {
        size_t struct_size;

        void (*destroy)(TF_Store* store);
        void (*get_name)(TF_Store* store, TF_String* out_name);
        void (*is_connected)(TF_Store* store, int* out_connected);

        // Operations.
        void (*backup)(TF_Store* store, const TF_String* destination, TFStoreAckFn completion, void* user_data, TF_Status* out_status);
        void (*restore)(TF_Store* store, const TF_String* source, TFStoreAckFn completion, void* user_data, TF_Status* out_status);

    } TF_StoreOps;

#define TF_STORE_STRUCT_SIZE TF_OFFSET_OF_END(TF_StoreOps, restore)

    TF_CAPI_EXPORT void create_store(TF_StoreOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_store(void* plugin_context);

    static inline void init_store(TF_StoreOps** ops, TF_Store* store, TF_Status* out_status)
    {
        create_store(ops, &store->plugin_data, out_status);

        TFStoreCollectionOps* collection_ops = NULL;
        create_store_collection(&collection_ops, &store->plugin_data, out_status);
        store->collection_ops = collection_ops;

        TFStoreTransactionOps* transaction_ops = NULL;
        create_store_transaction(&transaction_ops, &store->plugin_data, out_status);
        store->transaction_ops = transaction_ops;

        TFStoreWatchOps* watch_ops = NULL;
        create_store_watch(&watch_ops, &store->plugin_data, out_status);
        store->watch_ops = watch_ops;

        TFStoreIndexOps* index_ops = NULL;
        create_store_index(&index_ops, &store->plugin_data, out_status);
        store->index_ops = index_ops;

        TFStoreQueryOps* query_ops = NULL;
        create_store_query(&query_ops, &store->plugin_data, out_status);
        store->query_ops = query_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_STORE_H_
