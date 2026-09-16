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

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_Buffer_Data — passive value type holding a pointer to a block of data and its length.  Typically the data is a serialised protocol buffer. By default TF_Buffer_Data does not manage the pointed-to memory; set data_deallocator if the block needs freeing.
    typedef struct TF_Buffer_Data
    {
        const void* data;
        size_t length;
        void (*data_deallocator)(void* data, size_t length);
    } TF_Buffer_Data;

    // Legacy alias kept so existing call sites that use TF_Buffer as a value type continue to compile without changes.
    typedef TF_Buffer_Data TF_Buffer_Value;

    // Global helper functions that operate on TF_Buffer_Data values.
    TF_CAPI_EXPORT TF_Buffer_Data* new_buffer_from_string(const void* proto, size_t proto_len);
    TF_CAPI_EXPORT TF_Buffer_Data* new_buffer(void);
    TF_CAPI_EXPORT void delete_buffer(TF_Buffer_Data*);
    TF_CAPI_EXPORT const TF_Buffer_Data* get_buffer(const TF_Buffer_Data* buffer);

    // TF_Buffer — plugin vtable for buffer operations.
    typedef struct TF_BufferOps TF_BufferOps;

    typedef struct TF_Buffer
    {
        void* plugin_data;
        const TF_BufferOps* ops;
    } TF_Buffer;

    // Plugin-facing vtable registered via create_buffer so the mainframe can drive buffer operations across the C ABI.
    typedef struct TF_BufferOps
    {
        size_t struct_size;

        // Return the backend's name (e.g. "buffer") into *out.
        void (*get_name)(TF_Buffer* buffer, TF_String* out);

        // Copy proto[0..proto_len) into buffer, replacing any existing contents.
        void (*assign_from_string)(TF_Buffer* buffer, const void* proto, size_t proto_len);

        void (*delete_buffer)(TF_Buffer* buffer);

        // Return a non-owning TF_Buffer_Data view of the handle's contents.
        TF_Buffer_Data (*get_buffer)(TF_Buffer* buffer);

    } TF_BufferOps;

#define TF_BUFFER_STRUCT_SIZE TF_OFFSET_OF_END(TF_BufferOps, get_buffer)

    TF_CAPI_EXPORT void create_buffer(TF_BufferOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_buffer(void* plugin_context);

    // Real implementation, not declared-only — calls create_buffer and fills in buffer->ops.
    static inline void init_buffer(TF_Buffer* buffer, TF_Status* status)
    {
        TF_BufferOps* ops = NULL;
        create_buffer(&ops, &buffer->plugin_data, status);
        buffer->ops = ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_BUFFER_H_
