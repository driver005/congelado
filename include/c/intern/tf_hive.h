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

#ifndef TENSORFLOW_C_TF_HIVE_H_
#define TENSORFLOW_C_TF_HIVE_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Hive — plugin vtable for a type-erased stable-address bucket
    // container (C++26 std::hive/colony equivalent), fixed to element_size
    // bytes per element at creation. Insert/erase are O(1) and never
    // invalidate other elements' slot handles, unlike TF_Vector.
    //
    // TF_Hive_Handle is an opaque pointer to a plugin-owned hive object.
    typedef struct TF_Hive_Handle TF_Hive_Handle;

    // TF_Hive_Slot is an opaque, stable handle to one element within a
    // TF_Hive_Handle.
    typedef struct TF_Hive_Slot TF_Hive_Slot;

    // Callback type for iterating over a hive's live elements.
    typedef void (*TF_HiveVisitor)(void* capture, const void* element);

    // Plugin-facing vtable registered via init_hive.
    typedef struct TF_Hive
    {
        size_t struct_size;

        // Allocate a new, empty hive of element_size-byte elements. Must be
        // freed with destroy.
        TF_Hive_Handle* (*new_hive)(void* plugin_context, size_t element_size);

        // Copy one element_size-byte element from value into a newly
        // allocated slot, returning a stable handle to it.
        TF_Hive_Slot* (*insert)(void* plugin_context, TF_Hive_Handle* hive, const void* value);

        // Erase the element at slot, invalidating it.
        void (*erase)(void* plugin_context, TF_Hive_Handle* hive, TF_Hive_Slot* slot);

        // Non-owning pointer to the element at slot; NULL if slot has been
        // erased.
        const void* (*get)(void* plugin_context, const TF_Hive_Handle* hive, const TF_Hive_Slot* slot);

        // Call visitor(capture, element) once per live element, in
        // unspecified order.
        void (*for_each)(
            void* plugin_context,
            const TF_Hive_Handle* hive,
            TF_HiveVisitor visitor,
            void* capture
        );

        // Current live element count.
        size_t (*size)(void* plugin_context, const TF_Hive_Handle* hive);

        // Free a handle returned by new_hive.
        void (*destroy)(void* plugin_context, TF_Hive_Handle* hive);

    } TF_Hive;

#define TF_HIVE_STRUCT_SIZE TF_OFFSET_OF_END(TF_Hive, destroy)

    TF_CAPI_EXPORT void init_hive(TF_Hive** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_HIVE_H_
