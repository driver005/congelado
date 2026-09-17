#ifndef CONGELADO_C_STORE_TRANSACTION_H_
#define CONGELADO_C_STORE_TRANSACTION_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tensor.h"
#include "c/intern/tf_tstring.h"

#include "c/extern/store/collection.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Store_Transaction
    {
        void* plugin_data;
    } TF_Store_Transaction;

    typedef struct TF_Store_TransactionOps
    {
        size_t struct_size;
        void (*destroy)(TF_Store_Transaction* transaction);
        void (*begin)(TF_Store_Transaction* transaction, TF_Status* out_status);
        void (*add_collection)(TF_Store_Transaction* transaction, TF_Store_Collection* collection, TF_Status* out_status);
        void (*get_collection)(TF_Store_Transaction* transaction, const TF_String* name, TF_Store_Collection* out_collection);
        void (*list_collections)(TF_Store_Transaction* transaction, TF_Tensor** out_collections, TF_Status* out_status);
        void (*commit)(TF_Store_Transaction* transaction, TF_Store_AckFn completion, void* user_data, TF_Status* out_status);
        void (*rollback)(TF_Store_Transaction* transaction);
    } TF_Store_TransactionOps;

#define TF_STORE_TRANSACTION_STRUCT_SIZE TF_OFFSET_OF_END(TF_Store_TransactionOps, rollback)

    TF_CAPI_EXPORT void create_store_transaction(
        TF_Store_TransactionOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_store_transaction(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_STORE_TRANSACTION_H_
