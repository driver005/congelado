#ifndef CONGELADO_C_STORE_QUERY_H_
#define CONGELADO_C_STORE_QUERY_H_

#include "c/macros.h"
#include "c/intern/tf_map.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_vector.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*TF_Store_QueryFn)(void* user_data, const TF_Vector* matches, TF_Status* out_status);

    typedef struct TF_Store_Query
    {
        void* plugin_data;
    } TF_Store_Query;

    typedef struct TF_Store_QueryOps
    {
        size_t struct_size;
        void (*destroy)(TF_Store_Query* query);

        // filters: arbitrary field->value equality constraints (backend-specific, genuinely open-ended). free_text/sort/offset/limit: universal, explicitly typed. free_text and sort are nullable; limit == 0 means no limit.
        void (*run)(
            TF_Store_Query* query,
            const TF_Map* filters,
            const TF_String* free_text,
            const TF_String* sort,
            size_t offset,
            size_t limit,
            TF_Store_QueryFn completion,
            void* user_data,
            TF_Status* out_status
        );
    } TF_Store_QueryOps;

#define TF_STORE_QUERY_STRUCT_SIZE TF_OFFSET_OF_END(TF_Store_QueryOps, run)

    TF_CAPI_EXPORT void create_store_query(
        TF_Store_QueryOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_store_query(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_STORE_QUERY_H_
