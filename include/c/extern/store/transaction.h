#ifndef CONGELADO_C_STORE_TRANSACTION_H_
#define CONGELADO_C_STORE_TRANSACTION_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

#include "include/c/extern/store/collection.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFStoreTransaction
    {
        void* plugin_data;
    } TFStoreTransaction;

    typedef struct TFStoreTransactionOps
    {
        size_t struct_size;
        void (*destroy)(TFStoreTransaction* transaction);
        void (*begin)(TFStoreTransaction* transaction, TF_Status* out_status);
        void (*add_collection)(TFStoreTransaction* transaction, TFStoreCollection* collection, TF_Status* out_status);
        void (*get_collection)(TFStoreTransaction* transaction, const TF_String* name, TFStoreCollection* out_collection);
        void (*list_collections)(TFStoreTransaction* transaction, TF_Tensor** out_collections, TF_Status* out_status);
        void (*commit)(TFStoreTransaction* transaction, TFStoreAckFn completion, void* user_data, TF_Status* out_status);
        void (*rollback)(TFStoreTransaction* transaction);
    } TFStoreTransactionOps;

#define TF_STORE_TRANSACTION_STRUCT_SIZE TF_OFFSET_OF_END(TFStoreTransactionOps, rollback)

    TF_CAPI_EXPORT void create_store_transaction(
        TFStoreTransactionOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_store_transaction(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_STORE_TRANSACTION_H_
