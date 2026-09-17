#ifndef CONGELADO_C_STORE_INDEX_H_
#define CONGELADO_C_STORE_INDEX_H_

#include "include/c/macros.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFStoreIndex
    {
        void* plugin_data;
    } TFStoreIndex;

    typedef struct TFStoreIndexOps
    {
        size_t struct_size;
        void (*destroy)(TFStoreIndex* index);
        void (*create)(TFStoreIndex* index, const TF_String* name, const TF_Map* field_config, TF_Status* out_status);
        void (*drop)(TFStoreIndex* index, const TF_String* name, TF_Status* out_status);
        void (*list)(TFStoreIndex* index, TF_Vector* out_names, TF_Status* out_status);
    } TFStoreIndexOps;

#define TF_STORE_INDEX_STRUCT_SIZE TF_OFFSET_OF_END(TFStoreIndexOps, list)

    TF_CAPI_EXPORT void create_store_index(
        TFStoreIndexOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_store_index(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_STORE_INDEX_H_
