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

#ifndef TENSORFLOW_C_TF_DATATYPE_H_
#define TENSORFLOW_C_TF_DATATYPE_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_DataType_Enum — scalar element type for tensors.
    // Values are identical to the corresponding entries in types.proto.
    typedef enum TF_DataType_Enum
    {
        TF_FLOAT = 1,
        TF_DOUBLE = 2,
        TF_INT32 = 3,
        TF_UINT8 = 4,
        TF_INT16 = 5,
        TF_INT8 = 6,
        TF_STRING = 7,
        TF_COMPLEX64 = 8,
        TF_COMPLEX = 8, // backwards-compat alias
        TF_INT64 = 9,
        TF_BOOL = 10,
        TF_QINT8 = 11,
        TF_QUINT8 = 12,
        TF_QINT32 = 13,
        TF_BFLOAT16 = 14,
        TF_QINT16 = 15,
        TF_QUINT16 = 16,
        TF_UINT16 = 17,
        TF_COMPLEX128 = 18,
        TF_HALF = 19,
        TF_RESOURCE = 20,
        TF_VARIANT = 21,
        TF_UINT32 = 22,
        TF_UINT64 = 23,
        TF_FLOAT8_E5M2 = 24,
        TF_FLOAT8_E4M3FN = 25,
        TF_FLOAT8_E4M3FNUZ = 26,
        TF_FLOAT8_E4M3B11FNUZ = 27,
        TF_FLOAT8_E5M2FNUZ = 28,
        TF_INT4 = 29,
        TF_UINT4 = 30,
        TF_INT2 = 31,
        TF_UINT2 = 32,
        TF_FLOAT4_E2M1FN = 33
    } TF_DataType_Enum;

    // Legacy alias — existing code that spells the enum as TF_DataType continues
    // to compile without changes.
    typedef TF_DataType_Enum TF_DataType;

    // Global helper (non-vtable path).
    TF_CAPI_EXPORT size_t data_type_size(TF_DataType_Enum dt);

    // --------------------------------------------------------------------------
    // TF_DataType — plugin vtable for data-type operations.
    typedef struct TF_DataTypeOps
    {
        size_t struct_size;

        // Return the backend's name (e.g. "datatype") into *out.
        void (*get_name)(void* plugin_context, TF_String* out);

        // Return the byte size of one scalar element of the given type.
        // Returns 0 for variable-length types (e.g. TF_STRING) or on failure.
        size_t (*data_type_size)(void* plugin_context, TF_DataType_Enum dt);

    } TF_DataTypeOps;

#define TF_DATATYPE_STRUCT_SIZE TF_OFFSET_OF_END(TF_DataTypeOps, data_type_size)

    TF_CAPI_EXPORT void init_data_type(TF_DataType** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_DATATYPE_H_
