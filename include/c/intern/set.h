#ifndef TENSORFLOW_C_TF_SET_H_
#define TENSORFLOW_C_TF_SET_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef size_t (*TF_SetHashFn)(const void* key, size_t key_size);
    typedef int (*TF_SetCompareFn)(const void* lhs, const void* rhs, size_t key_size);
    typedef void (*TF_SetVisitor)(void* capture, const void* key);

    typedef struct TF_Set
    {
        void* plugin_data;
    } TF_Set;

    typedef struct TF_SetOps
    {
        size_t struct_size;

        void (*insert)(TF_Set* set, const void* key, TF_Status* out_status);
        void (*find)(const TF_Set* set, const void* key, const void** out_value, TF_Status* out_status);
        void (*erase)(TF_Set* set, const void* key, TF_Status* out_status);
        void (*contains)(const TF_Set* set, const void* key, int* out_found, TF_Status* out_status);
        void (*size)(const TF_Set* set, size_t* out_size);
        void (*for_each)(const TF_Set* set, TF_SetVisitor visitor, void* capture);
        void (*destroy)(TF_Set* set);

    } TF_SetOps;

#define TF_SET_STRUCT_SIZE TF_OFFSET_OF_END(TF_SetOps, destroy)

    TF_CAPI_EXPORT void create_set(TF_SetOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_set(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_SET_H_
