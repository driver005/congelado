#ifndef TENSORFLOW_C_TF_BITSET_H_
#define TENSORFLOW_C_TF_BITSET_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

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
        int (*test)(const TF_BitSet* bitset, size_t index);
        void (*flip)(TF_BitSet* bitset, size_t index);
        size_t (*count)(const TF_BitSet* bitset);
        size_t (*size)(const TF_BitSet* bitset);
        void (*destroy)(TF_BitSet* bitset);

    } TF_BitSetOps;

#define TF_BITSET_STRUCT_SIZE TF_OFFSET_OF_END(TF_BitSetOps, destroy)

    TF_CAPI_EXPORT void create_bitset(TF_BitSetOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_bitset(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_BITSET_H_
