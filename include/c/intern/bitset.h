#ifndef TENSORFLOW_C_TF_BITSET_H_
#define TENSORFLOW_C_TF_BITSET_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_BitSet
    {
        void* plugin_data;
    } TF_BitSet;

    typedef struct TF_BitSetOps
    {
        size_t struct_size;

        void (*set)(TF_BitSet* bitset, size_t index);
        void (*clear)(TF_BitSet* bitset, size_t index);
        void (*test)(const TF_BitSet* bitset, size_t index, int* out_result, TF_Status* out_status);
        void (*flip)(TF_BitSet* bitset, size_t index, TF_Status* out_status);
        void (*count)(const TF_BitSet* bitset, size_t* out_count);
        void (*size)(const TF_BitSet* bitset, size_t* out_size);
        void (*destroy)(TF_BitSet* bitset);

    } TF_BitSetOps;

#define TF_BITSET_STRUCT_SIZE TF_OFFSET_OF_END(TF_BitSetOps, destroy)

    TF_CAPI_EXPORT void create_bitset(TF_BitSetOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_bitset(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_BITSET_H_
