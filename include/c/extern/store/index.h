#ifndef CONGELADO_C_STORE_INDEX_H_
#define CONGELADO_C_STORE_INDEX_H_

#include "c/macros.h"
#include "c/intern/tf_map.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_vector.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Store_Index
    {
        void* plugin_data;
    } TF_Store_Index;

    typedef struct TF_Store_IndexOps
    {
        size_t struct_size;
        void (*destroy)(TF_Store_Index* index);
        void (*create)(TF_Store_Index* index, const TF_String* name, const TF_Map* field_config, TF_Status* out_status);
        void (*drop)(TF_Store_Index* index, const TF_String* name, TF_Status* out_status);
        void (*list)(TF_Store_Index* index, TF_Vector* out_names, TF_Status* out_status);
    } TF_Store_IndexOps;

#define TF_STORE_INDEX_STRUCT_SIZE TF_OFFSET_OF_END(TF_Store_IndexOps, list)

    TF_CAPI_EXPORT void create_store_index(
        TF_Store_IndexOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_store_index(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_STORE_INDEX_H_
