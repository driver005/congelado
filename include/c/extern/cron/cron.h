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
#ifndef CONGELADO_C_CRON_CONTROLLER_H_
#define CONGELADO_C_CRON_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_bool.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    
    

    

    
    
    
    
    

    typedef struct TF_Cron {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        TF_Bool (*validate)(void* plugin_context, const TF_TString* expression, TF_Status* status);
        TF_Bool (*next_after)(void* plugin_context, const TF_TString* expression, int64_t base_time_ms, int64_t* out_time_ms, TF_Status* status);
        void (*upsert_job)(void* plugin_context, const TF_TString* name, const TF_TString* expression, TF_Status* status);
        void (*remove_job)(void* plugin_context, const TF_TString* name, TF_Status* status);
    } TF_Cron;

    TF_CAPI_EXPORT extern void TF_InitCron(TF_Cron** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_CRON_CONTROLLER_H_
