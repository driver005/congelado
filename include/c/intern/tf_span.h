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

#ifndef TENSORFLOW_C_TF_SPAN_H_
#define TENSORFLOW_C_TF_SPAN_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Span — plugin vtable for a type-erased non-owning view over
    // contiguous memory (std::span<T> equivalent), fixed to element_size
    // bytes per element at creation. A span never allocates, copies, or frees
    // the memory it views; destroy only frees the lightweight handle itself.
    //
    // TF_Span_Handle is an opaque pointer to a plugin-owned span object.
    typedef struct TF_Span_Handle TF_Span_Handle;

    // Plugin-facing vtable registered via init_span.
    typedef struct TF_Span
    {
        size_t struct_size;

        // Wrap an existing, caller-owned block of count element_size-byte
        // elements starting at data. The block must outlive the span. Must
        // be freed with destroy.
        TF_Span_Handle* (*new_span)(
            void* plugin_context,
            void* data,
            size_t count,
            size_t element_size
        );

        // Non-owning pointer to the element at index; NULL if out of range.
        const void* (*get)(void* plugin_context, const TF_Span_Handle* span, size_t index);

        // Element count.
        size_t (*size)(void* plugin_context, const TF_Span_Handle* span);

        // Non-owning pointer to the viewed contiguous storage.
        void* (*data)(void* plugin_context, const TF_Span_Handle* span);

        // A new span viewing the same storage, starting at offset for count
        // elements. Must be freed independently with destroy.
        TF_Span_Handle* (*subspan)(
            void* plugin_context,
            const TF_Span_Handle* span,
            size_t offset,
            size_t count
        );

        // Free a handle returned by new_span or subspan. Never touches the
        // viewed memory.
        void (*destroy)(void* plugin_context, TF_Span_Handle* span);

    } TF_Span;

#define TF_SPAN_STRUCT_SIZE TF_OFFSET_OF_END(TF_Span, destroy)

    TF_CAPI_EXPORT void init_span(TF_Span** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_SPAN_H_
