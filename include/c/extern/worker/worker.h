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
#ifndef CONGELADO_C_WORKER_CONTROLLER_H_
#define CONGELADO_C_WORKER_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif



    typedef void (*TF_Worker_CompletionFn)(const TF_TString* result_json, void* user_data);

    typedef struct TF_Worker {
        size_t struct_size;
        void (*get_name)(void* plugin_context, TF_String* out);
        void (*get_task_type)(void* plugin_context, TF_String* out, TF_Status* status);
        void (*execute)(
            void* plugin_context, const TF_TString* input_json,
            TF_String* out_result_json, TF_Status* status
        );
        void (*execute_async)(
            void* plugin_context, const TF_TString* input_json,
            TF_Worker_CompletionFn completion, void* cb_user_data, TF_Status* status
        );
        void (*on_error)(
            void* plugin_context, const TF_TString* message, TF_Status* status
        );
        void (*on_released)(void* plugin_context, TF_Status* status);
        void (*destroy)(void* plugin_context);
    } TF_Worker;

    TF_CAPI_EXPORT extern void TF_InitWorker(TF_Worker** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_WORKER_CONTROLLER_H_
