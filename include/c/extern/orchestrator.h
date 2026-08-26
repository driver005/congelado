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
#ifndef CONGELADO_C_ORCHESTRATOR_H_
#define CONGELADO_C_ORCHESTRATOR_H_

#include "c/abi/macros.h"
#include "c/extern/worker.h"
#include "c/intern/tf_bool.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
typedef struct TP_Orchestrator TP_Orchestrator;

#ifdef __cplusplus
extern "C"
{
#endif

    typedef TF_Bool (*TP_Orchestrator_RequiredFn)(void* user_data);
    typedef void (*TP_Orchestrator_StartWorkflowFn)(
        void* user_data,
        const TF_TString* def_name,
        const TF_TString* variables_json,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    );
    typedef void (*TP_Orchestrator_PauseFn)(
        void* user_data,
        const TF_TString* exec_id,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    );
    typedef void (*TP_Orchestrator_ResumeFn)(
        void* user_data,
        const TF_TString* exec_id,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    );
    typedef void (*TP_Orchestrator_TerminateFn)(
        void* user_data,
        const TF_TString* exec_id,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    );
    typedef void (*TP_Orchestrator_CompleteTaskFn)(
        void* user_data,
        const TF_TString* exec_id,
        const TF_TString* node_ref,
        TF_Bool success,
        const TF_TString* output_json,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    );

    typedef void (*TF_OrchestratorRegistrationParams_DestroyOrchestrator)(TP_Orchestrator* orch);

    typedef struct TP_Orchestrator
    {
        size_t struct_size;
        void* ext;
        TF_TString backend_name;
        TP_Orchestrator_RequiredFn required_cb;
        TP_Orchestrator_StartWorkflowFn start_workflow_cb;
        TP_Orchestrator_PauseFn pause_cb;
        TP_Orchestrator_ResumeFn resume_cb;
        TP_Orchestrator_TerminateFn terminate_cb;
        TP_Orchestrator_CompleteTaskFn complete_task_cb;
#define TP_ORCHESTRATOR_STRUCT_SIZE TF_OFFSET_OF_END(TP_Orchestrator, complete_task_cb)
    } TP_Orchestrator;

    typedef struct TF_OrchestratorRegistrationParams
    {
        size_t struct_size;
        void* ext;
        int32_t major_version;
        int32_t minor_version;
        int32_t patch_version;
        TP_Orchestrator* orchestrator;
        TF_OrchestratorRegistrationParams_DestroyOrchestrator destroy_orchestrator;
#define TF_ORCHESTRATOR_REGISTRATION_PARAMS_STRUCT_SIZE                                            \
    TF_OFFSET_OF_END(TF_OrchestratorRegistrationParams, destroy_orchestrator)
    } TF_OrchestratorRegistrationParams;

    static inline TP_Orchestrator* TP_OrchestratorNew(void)
    {
        TP_Orchestrator* ptr = (TP_Orchestrator*)malloc(TP_ORCHESTRATOR_STRUCT_SIZE);
        if (!ptr) {
            return nullptr;
        }
        ptr->struct_size = TP_ORCHESTRATOR_STRUCT_SIZE;
        ptr->ext = nullptr;
        TF_StringInit(&ptr->backend_name);
        ptr->required_cb = nullptr;
        ptr->start_workflow_cb = nullptr;
        ptr->pause_cb = nullptr;
        ptr->resume_cb = nullptr;
        ptr->terminate_cb = nullptr;
        ptr->complete_task_cb = nullptr;
        return ptr;
    }

    static inline void TP_OrchestratorDelete(TP_Orchestrator* ptr)
    {
        if (!ptr) {
            return;
        }
        TF_StringDealloc(&ptr->backend_name);
        free(ptr);
    }

    static inline void
    TP_Orchestrator_SetBackendName(TP_Orchestrator* builder, TF_TString backend_name)
    {
        builder->backend_name = backend_name;
    }

    static inline void TP_Orchestrator_SetRequiredCallback(
        TP_Orchestrator* builder, TP_Orchestrator_RequiredFn required_cb
    )
    {
        builder->required_cb = required_cb;
    }

    static inline void TP_Orchestrator_SetStartWorkflowCallback(
        TP_Orchestrator* builder, TP_Orchestrator_StartWorkflowFn start_workflow_cb
    )
    {
        builder->start_workflow_cb = start_workflow_cb;
    }

    static inline void
    TP_Orchestrator_SetPauseCallback(TP_Orchestrator* builder, TP_Orchestrator_PauseFn pause_cb)
    {
        builder->pause_cb = pause_cb;
    }

    static inline void
    TP_Orchestrator_SetResumeCallback(TP_Orchestrator* builder, TP_Orchestrator_ResumeFn resume_cb)
    {
        builder->resume_cb = resume_cb;
    }

    static inline void TP_Orchestrator_SetTerminateCallback(
        TP_Orchestrator* builder, TP_Orchestrator_TerminateFn terminate_cb
    )
    {
        builder->terminate_cb = terminate_cb;
    }

    static inline void TP_Orchestrator_SetCompleteTaskCallback(
        TP_Orchestrator* builder, TP_Orchestrator_CompleteTaskFn complete_task_cb
    )
    {
        builder->complete_task_cb = complete_task_cb;
    }

#define OR_MAJOR 0
#define OR_MINOR 0
#define OR_PATCH 1

    void TF_InitOrchestrator(TF_OrchestratorRegistrationParams* params, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif
