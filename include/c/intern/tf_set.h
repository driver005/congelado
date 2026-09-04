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

#ifndef TENSORFLOW_C_TF_SET_H_
#define TENSORFLOW_C_TF_SET_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Set — plugin vtable for a type-erased key-only associative
    // container. A single TF_Set type collapses std::set/std::multiset/
    // std::unordered_set/std::unordered_multiset — ordered vs. hashed storage
    // is a matter of which init_set implementation is registered, and
    // allow_duplicates controls multi-key behavior, rather than distinct C
    // types per STL variant. Same hash/compare convention as TF_Map.
    typedef size_t (*TF_SetHashFn)(const void* key, size_t key_size);

    typedef int (*TF_SetCompareFn)(const void* lhs, const void* rhs, size_t key_size);

    // Callback type for iterating over a set's elements.
    typedef void (*TF_SetVisitor)(void* capture, const void* key);

    // TF_Set_Handle is an opaque pointer to a plugin-owned set object.
    typedef struct TF_Set_Handle TF_Set_Handle;

    // Plugin-facing vtable registered via init_set.
    typedef struct TF_Set
    {
        size_t struct_size;

        // Allocate a new, empty set of key_size-byte keys. hash_fn/
        // compare_fn are used for every subsequent operation on the
        // returned handle. allow_duplicates permits multiple equal keys
        // (multiset semantics). Must be freed with destroy.
        TF_Set_Handle* (*new_set)(
            void* plugin_context,
            size_t key_size,
            TF_SetHashFn hash_fn,
            TF_SetCompareFn compare_fn,
            int allow_duplicates
        );

        // Copy one key_size-byte key in. Non-zero on success.
        int (*insert)(void* plugin_context, TF_Set_Handle* set, const void* key);

        // Non-owning pointer to the stored key equal to key; NULL if absent.
        const void* (*find)(void* plugin_context, const TF_Set_Handle* set, const void* key);

        // Remove the element (or elements, if allow_duplicates) equal to
        // key. Non-zero if anything was removed.
        int (*erase)(void* plugin_context, TF_Set_Handle* set, const void* key);

        // Non-zero if an element equal to key exists.
        int (*contains)(void* plugin_context, const TF_Set_Handle* set, const void* key);

        // Current element count.
        size_t (*size)(void* plugin_context, const TF_Set_Handle* set);

        // Call visitor(capture, key) once per element, in unspecified order.
        void (*for_each)(
            void* plugin_context,
            const TF_Set_Handle* set,
            TF_SetVisitor visitor,
            void* capture
        );

        // Free a handle returned by new_set.
        void (*destroy)(void* plugin_context, TF_Set_Handle* set);

    } TF_Set;

#define TF_SET_STRUCT_SIZE TF_OFFSET_OF_END(TF_Set, destroy)

    TF_CAPI_EXPORT void init_set(TF_Set** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_SET_H_
