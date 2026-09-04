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

#ifndef TENSORFLOW_C_TF_VECTOR_H_
#define TENSORFLOW_C_TF_VECTOR_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Vector — plugin vtable for a type-erased growable, contiguous
    // collection (std::vector<T> equivalent), fixed to element_size bytes per
    // element at creation.
    //
    // TF_Vector_Handle is an opaque pointer to a plugin-owned vector object.
    typedef struct TF_Vector_Handle TF_Vector_Handle;

    // Plugin-facing vtable registered via init_vector.
    typedef struct TF_Vector
    {
        size_t struct_size;

        // Allocate a new, empty vector of element_size-byte elements. Must be
        // freed with destroy.
        TF_Vector_Handle* (*new_vector)(void* plugin_context, size_t element_size);

        // Copy one element_size-byte element from value onto the end.
        void (*push_back)(void* plugin_context, TF_Vector_Handle* vector, const void* value);

        // Non-owning pointer to the element at index; NULL if out of range.
        const void* (*get)(void* plugin_context, const TF_Vector_Handle* vector, size_t index);

        // Copy one element_size-byte element from value over the element at
        // index.
        void (*set)(
            void* plugin_context,
            TF_Vector_Handle* vector,
            size_t index,
            const void* value
        );

        // Current element count.
        size_t (*size)(void* plugin_context, const TF_Vector_Handle* vector);

        // Current storage capacity in elements.
        size_t (*capacity)(void* plugin_context, const TF_Vector_Handle* vector);

        // Ensure capacity for at least new_capacity elements.
        void (*reserve)(void* plugin_context, TF_Vector_Handle* vector, size_t new_capacity);

        // Non-owning pointer to the contiguous backing storage.
        void* (*data)(void* plugin_context, TF_Vector_Handle* vector);

        // Free a handle returned by new_vector.
        void (*destroy)(void* plugin_context, TF_Vector_Handle* vector);

    } TF_Vector;

#define TF_VECTOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_Vector, destroy)

    TF_CAPI_EXPORT void init_vector(TF_Vector** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_VECTOR_H_
