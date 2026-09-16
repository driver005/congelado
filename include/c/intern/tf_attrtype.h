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

#ifndef TENSORFLOW_C_TF_ATTRTYPE_H_
#define TENSORFLOW_C_TF_ATTRTYPE_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_AttrType_Enum describes the type of the value of an attribute on an operation.
    typedef enum TF_AttrType_Enum
    {
        TF_ATTR_STRING = 0,
        TF_ATTR_INT = 1,
        TF_ATTR_FLOAT = 2,
        TF_ATTR_BOOL = 3,
        TF_ATTR_TYPE = 4,
        TF_ATTR_SHAPE = 5,
        TF_ATTR_TENSOR = 6,
        TF_ATTR_PLACEHOLDER = 7,
        TF_ATTR_FUNC = 8,
    } TF_AttrType_Enum;

    // --------------------------------------------------------------------------
    // TF_AttrType — plugin vtable for attr-type operations.
    typedef struct TF_AttrType
    {
        size_t struct_size;

        // Return the backend's name (e.g. "attrtype") into *out.
        void (*get_name)(void* plugin_context, TF_String* out);

        // Return the human-readable name of the given attr type (e.g. "string" for
        // TF_ATTR_STRING) into *out.
        void (*attrtype_name)(void* plugin_context, TF_AttrType_Enum type, TF_String* out);

    } TF_AttrType;

#define TF_ATTRTYPE_STRUCT_SIZE TF_OFFSET_OF_END(TF_AttrType, attrtype_name)

    TF_CAPI_EXPORT void
    init_attrtype(TF_AttrType** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_ATTRTYPE_H_
