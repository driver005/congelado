/* Copyright 2024 The Congelado Authors. All Rights Reserved.

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
#ifndef CONGELADO_C_SERDE_CONTROLLER_H_
#define CONGELADO_C_SERDE_CONTROLLER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Serde
    {
        void* plugin_data;
    } TF_Serde;

    typedef struct TF_SerdeOps
    {
        size_t struct_size;
        void (*destroy)(TF_Serde* serde);
        void (*get_name)(TF_Serde* serde, TF_String* out_name);
        void (*get_content_type)(TF_Serde* serde, TF_String* out_content_type);
        void (*get_format_name)(TF_Serde* serde, TF_String* out_format_name);
        void (*encode)(
            TF_Serde* serde,
            const TF_String* value_json,
            TF_String* out_encoded,
            TF_Status* out_status
        );
        void (*decode)(
            TF_Serde* serde,
            const TF_String* data,
            TF_String* out_json,
            TF_Status* out_status
        );
    } TF_SerdeOps;

#define TF_SERDE_STRUCT_SIZE TF_OFFSET_OF_END(TF_SerdeOps, decode)

    TF_CAPI_EXPORT void create_serde(TF_SerdeOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_serde(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_SERDE_CONTROLLER_H_
