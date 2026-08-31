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

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif


    // encode/decode write their result into a caller-supplied TF_TString (by-value output
    // parameter).  No heap allocation, no free_string needed.

    typedef struct TF_Serde
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        void (*get_content_type)(void* plugin_context, TF_String* out);
        void (*get_format_name)(void* plugin_context, TF_String* out);
        void (*encode)(
            void* plugin_context,
            const TF_TString* value_json,
            TF_String* out_encoded,
            TF_Status* status
        );
        void (*decode)(
            void* plugin_context,
            const TF_TString* data,
            TF_String* out_json,
            TF_Status* status
        );
    } TF_Serde;

#define TF_SERDE_STRUCT_SIZE TF_OFFSET_OF_END(TF_Serde, decode)

    TF_CAPI_EXPORT void init_serde(TF_Serde** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_SERDE_CONTROLLER_H_
