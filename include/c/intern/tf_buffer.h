/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

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

#ifndef TENSORFLOW_C_TF_BUFFER_H_
#define TENSORFLOW_C_TF_BUFFER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Buffer_Data — passive value type holding a pointer to a block of data
    // and its length.  Typically the data is a serialised protocol buffer.
    // By default TF_Buffer_Data does not manage the pointed-to memory; set
    // data_deallocator if the block needs freeing.
    typedef struct TF_Buffer_Data
    {
        const void* data;
        size_t length;
        void (*data_deallocator)(void* data, size_t length);
    } TF_Buffer_Data;

    // Legacy alias kept so existing call sites that use TF_Buffer as a value
    // type continue to compile without changes.
    typedef TF_Buffer_Data TF_Buffer_Value;

    // Global helper functions that operate on TF_Buffer_Data values.
    TF_CAPI_EXPORT extern TF_Buffer_Data* TF_NewBufferFromString(const void* proto, size_t proto_len);
    TF_CAPI_EXPORT extern TF_Buffer_Data* TF_NewBuffer(void);
    TF_CAPI_EXPORT extern void           TF_DeleteBuffer(TF_Buffer_Data*);
    TF_CAPI_EXPORT extern TF_Buffer_Data TF_GetBuffer(TF_Buffer_Data* buffer);

    // --------------------------------------------------------------------------
    // TF_Buffer — plugin vtable for buffer operations.
    //
    // TF_Buffer_Handle is an opaque pointer to a plugin-owned buffer object.
    typedef struct TF_Buffer_Handle TF_Buffer_Handle;

    // Plugin-facing vtable registered via TF_InitBuffer so the mainframe can
    // drive buffer operations across the C ABI.
    typedef struct TF_Buffer
    {
        size_t struct_size;

        // Allocate a new buffer copied from proto[0..proto_len).
        // Must be freed with the destroy field below or TF_DeleteBuffer.
        TF_Buffer_Handle* (*TF_NewBufferFromString)(
            void* ctx, const void* proto, size_t proto_len
        );

        // Allocate a new, empty buffer.
        TF_Buffer_Handle* (*TF_NewBuffer)(void* ctx);

        // Free a handle returned by the two factory functions above.
        void (*TF_DeleteBuffer)(void* ctx, TF_Buffer_Handle* buffer);

        // Return a non-owning TF_Buffer_Data view of the handle's contents.
        TF_Buffer_Handle (*TF_GetBuffer)(void* ctx, TF_Buffer_Handle* buffer);

    } TF_Buffer;

#define TF_BUFFER_STRUCT_SIZE TF_OFFSET_OF_END(TF_Buffer, TF_GetBuffer)

    TF_CAPI_EXPORT extern void TF_InitBuffer(
        TF_Buffer** ops, void** plugin_context, TF_Status* status
    );

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_BUFFER_H_
