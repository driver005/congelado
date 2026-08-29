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
#ifndef CONGELADO_C_ORCHESTRATOR_CONTROLLER_H_
#define CONGELADO_C_ORCHESTRATOR_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/extern/worker/worker.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif


    typedef struct TF_Orchestrator
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        void (*start_workflow)(
            void* plugin_context,
            const TF_TString* def_name,
            const TF_TString* variables_json,
            TF_Worker_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
        void (*pause)(
            void* plugin_context,
            const TF_TString* exec_id,
            TF_Worker_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
        void (*resume)(
            void* plugin_context,
            const TF_TString* exec_id,
            TF_Worker_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
        void (*terminate)(
            void* plugin_context,
            const TF_TString* exec_id,
            TF_Worker_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
        void (*complete_task)(
            void* plugin_context,
            const TF_TString* exec_id,
            const TF_TString* node_ref,
            bool success,
            const TF_TString* output_json,
            TF_Worker_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
    } TF_Orchestrator;

#define TF_ORCHESTRATOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_Orchestrator, complete_task)

    TF_CAPI_EXPORT void
    init_orchestrator(TF_Orchestrator** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_ORCHESTRATOR_CONTROLLER_H_
