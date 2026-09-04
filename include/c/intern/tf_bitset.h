/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_C_TF_BITSET_H_
#define TENSORFLOW_C_TF_BITSET_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_BitSet — plugin vtable for a type-erased fixed-length bit sequence
    // (std::bitset<N> equivalent), fixed to bit_count bits at creation.
    //
    // TF_BitSet_Handle is an opaque pointer to a plugin-owned bitset object.
    typedef struct TF_BitSet_Handle TF_BitSet_Handle;

    // Plugin-facing vtable registered via init_bitset.
    typedef struct TF_BitSet
    {
        size_t struct_size;

        // Allocate a new bitset of bit_count bits, all initially clear.
        // Must be freed with destroy.
        TF_BitSet_Handle* (*new_bitset)(void* plugin_context, size_t bit_count);

        // Set the bit at index to 1.
        void (*set)(void* plugin_context, TF_BitSet_Handle* bitset, size_t index);

        // Set the bit at index to 0.
        void (*clear)(void* plugin_context, TF_BitSet_Handle* bitset, size_t index);

        // Non-zero if the bit at index is set.
        int (*test)(void* plugin_context, const TF_BitSet_Handle* bitset, size_t index);

        // Invert the bit at index.
        void (*flip)(void* plugin_context, TF_BitSet_Handle* bitset, size_t index);

        // Number of set bits.
        size_t (*count)(void* plugin_context, const TF_BitSet_Handle* bitset);

        // Fixed bit count, as given to new_bitset.
        size_t (*size)(void* plugin_context, const TF_BitSet_Handle* bitset);

        // Free a handle returned by new_bitset.
        void (*destroy)(void* plugin_context, TF_BitSet_Handle* bitset);

    } TF_BitSet;

#define TF_BITSET_STRUCT_SIZE TF_OFFSET_OF_END(TF_BitSet, destroy)

    TF_CAPI_EXPORT void init_bitset(TF_BitSet** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_BITSET_H_
