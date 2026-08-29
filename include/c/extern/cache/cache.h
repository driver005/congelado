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
#ifndef CONGELADO_C_CACHE_CONTROLLER_H_
#define CONGELADO_C_CACHE_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Callback for async cache operations.
    // result: null-terminated string result, or NULL if not found.
    // user_data: opaque pointer passed through from the caller.
    typedef void (*TF_Cache_CompletionFn)(const TF_TString* result, void* user_data);

    
    

    

    
    
    

    typedef struct TF_Cache {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        void (*get)(void* plugin_context, const TF_TString* key, TF_Cache_CompletionFn completion, void* cb_user_data, TF_Status* status);
        void (*set)(void* plugin_context, const TF_TString* key, const TF_TString* value, TF_Cache_CompletionFn completion, void* cb_user_data, TF_Status* status);
        void (*remove)(void* plugin_context, const TF_TString* key, TF_Cache_CompletionFn completion, void* cb_user_data, TF_Status* status);
    } TF_Cache;

    TF_CAPI_EXPORT extern void TF_InitCache(TF_Cache** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_CACHE_CONTROLLER_H_
