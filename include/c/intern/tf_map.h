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

#ifndef TENSORFLOW_C_TF_MAP_H_
#define TENSORFLOW_C_TF_MAP_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Map — plugin vtable for a type-erased key-to-value associative
    // container. A single TF_Map type collapses std::map/std::multimap/
    // std::unordered_map/std::unordered_multimap: ordered vs. hashed storage
    // is a matter of which init_map implementation is registered, and
    // allow_duplicates controls multi-key behavior, rather than distinct C
    // types per STL variant.
    //
    // Caller-supplied hash_fn/compare_fn let a type-erased map hash/compare
    // keys it has no static type information about (hash_fn may be NULL for
    // an ordered-only implementation; compare_fn is always required).
    typedef size_t (*TF_MapHashFn)(const void* key, size_t key_size);

    // Three-way comparator: negative if lhs < rhs, 0 if equal, positive if
    // lhs > rhs. Used both for ordering and, via the ==0 case, for equality.
    typedef int (*TF_MapCompareFn)(const void* lhs, const void* rhs, size_t key_size);

    // Callback type for iterating over a map's entries.
    typedef void (*TF_MapVisitor)(void* capture, const void* key, const void* value);

    // TF_Map_Handle is an opaque pointer to a plugin-owned map object.
    typedef struct TF_Map_Handle TF_Map_Handle;

    // Plugin-facing vtable registered via init_map.
    typedef struct TF_Map
    {
        size_t struct_size;

        // Allocate a new, empty map of key_size-byte keys mapped to
        // value_size-byte values. hash_fn/compare_fn are used for every
        // subsequent operation on the returned handle. allow_duplicates
        // permits multiple entries under the same key (multimap semantics).
        // Must be freed with destroy.
        TF_Map_Handle* (*new_map)(
            void* plugin_context,
            size_t key_size,
            size_t value_size,
            TF_MapHashFn hash_fn,
            TF_MapCompareFn compare_fn,
            int allow_duplicates
        );

        // Copy one key_size-byte key and one value_size-byte value in.
        // Non-zero on success.
        int (*insert)(void* plugin_context, TF_Map_Handle* map, const void* key, const void* value);

        // Non-owning pointer to the value stored under key; NULL if absent.
        const void* (*find)(void* plugin_context, const TF_Map_Handle* map, const void* key);

        // Remove the entry (or entries, if allow_duplicates) stored under
        // key. Non-zero if anything was removed.
        int (*erase)(void* plugin_context, TF_Map_Handle* map, const void* key);

        // Non-zero if an entry exists under key.
        int (*contains)(void* plugin_context, const TF_Map_Handle* map, const void* key);

        // Current entry count.
        size_t (*size)(void* plugin_context, const TF_Map_Handle* map);

        // Call visitor(capture, key, value) once per entry, in unspecified
        // order.
        void (*for_each)(
            void* plugin_context,
            const TF_Map_Handle* map,
            TF_MapVisitor visitor,
            void* capture
        );

        // Free a handle returned by new_map.
        void (*destroy)(void* plugin_context, TF_Map_Handle* map);

    } TF_Map;

#define TF_MAP_STRUCT_SIZE TF_OFFSET_OF_END(TF_Map, destroy)

    TF_CAPI_EXPORT void init_map(TF_Map** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_MAP_H_
