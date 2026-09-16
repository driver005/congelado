#ifndef TENSORFLOW_C_TF_BITSET_H_
#define TENSORFLOW_C_TF_BITSET_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_BitSet — plugin vtable for a type-erased fixed-length bit sequence (std::bitset<N> equivalent), fixed to bit_count bits at creation.
    //
    // TF_BitSet is an opaque pointer to a plugin-owned bitset object.
    typedef struct TF_BitSet
    {
        void* plugin_data;
    } TF_BitSet;

    // Plugin-facing vtable registered via create_bitset.
    typedef struct TF_BitSetOps
    {
        size_t struct_size;

        // Allocate a new bitset of bit_count bits, all initially clear. Must be freed with destroy.
        TF_BitSet* (*new_bitset)(void* plugin_context, size_t bit_count);

        // Set the bit at index to 1.
        void (*set)(TF_BitSet* bitset, size_t index);

        // Set the bit at index to 0.
        void (*clear)(TF_BitSet* bitset, size_t index);

        // Non-zero if the bit at index is set.
        int (*test)(const TF_BitSet* bitset, size_t index);

        // Invert the bit at index.
        void (*flip)(TF_BitSet* bitset, size_t index);

        // Number of set bits.
        size_t (*count)(const TF_BitSet* bitset);

        // Fixed bit count, as given to new_bitset.
        size_t (*size)(const TF_BitSet* bitset);

        // Free a handle returned by new_bitset.
        void (*destroy)(TF_BitSet* bitset);

    } TF_BitSetOps;

#define TF_BITSET_STRUCT_SIZE TF_OFFSET_OF_END(TF_BitSetOps, destroy)

    TF_CAPI_EXPORT void create_bitset(TF_BitSetOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_bitset(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_BITSET_H_
