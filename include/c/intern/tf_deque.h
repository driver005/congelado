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

#ifndef TENSORFLOW_C_TF_DEQUE_H_
#define TENSORFLOW_C_TF_DEQUE_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Deque — plugin vtable for a type-erased double-ended growable
    // collection (std::deque<T> equivalent), fixed to element_size bytes per
    // element at creation.
    //
    // TF_Deque_Handle is an opaque pointer to a plugin-owned deque object.
    typedef struct TF_Deque_Handle TF_Deque_Handle;

    // Plugin-facing vtable registered via init_deque.
    typedef struct TF_Deque
    {
        size_t struct_size;

        // Allocate a new, empty deque of element_size-byte elements. Must be
        // freed with destroy.
        TF_Deque_Handle* (*new_deque)(void* plugin_context, size_t element_size);

        // Copy one element_size-byte element from value onto the front.
        void (*push_front)(void* plugin_context, TF_Deque_Handle* deque, const void* value);

        // Copy one element_size-byte element from value onto the back.
        void (*push_back)(void* plugin_context, TF_Deque_Handle* deque, const void* value);

        // Remove the front element. No-op if empty.
        void (*pop_front)(void* plugin_context, TF_Deque_Handle* deque);

        // Remove the back element. No-op if empty.
        void (*pop_back)(void* plugin_context, TF_Deque_Handle* deque);

        // Non-owning pointer to the element at index; NULL if out of range.
        const void* (*get)(void* plugin_context, const TF_Deque_Handle* deque, size_t index);

        // Current element count.
        size_t (*size)(void* plugin_context, const TF_Deque_Handle* deque);

        // Free a handle returned by new_deque.
        void (*destroy)(void* plugin_context, TF_Deque_Handle* deque);

    } TF_Deque;

#define TF_DEQUE_STRUCT_SIZE TF_OFFSET_OF_END(TF_Deque, destroy)

    TF_CAPI_EXPORT void init_deque(TF_Deque** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_DEQUE_H_
