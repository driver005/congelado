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
#ifndef CONGELADO_C_WORKER_H_
#define CONGELADO_C_WORKER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_bool.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
typedef struct TP_Worker TP_Worker;

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // Callback types.

    typedef void (*TF_Worker_CompletionFn)(const TF_TString* result_json, void* user_data);

    typedef const TF_TString* (*TP_Worker_GetTaskTypeFn)(void* user_data);
    typedef TF_Status* (*TP_Worker_ExecuteFn)(
        void* user_data, const TF_TString* input_json, TF_TString** out_result_json
    );
    typedef void (*TP_Worker_ExecuteAsyncFn)(
        void* user_data,
        const TF_TString* input_json,
        TF_Worker_CompletionFn completion,
        void* cb_user_data
    );
    typedef void (*TP_Worker_OnErrorFn)(void* user_data, const TF_TString* message);
    typedef void (*TP_Worker_OnReleasedFn)(void* user_data);

    typedef void (*TF_WorkerRegistrationParams_DestroyWorker)(TP_Worker* worker);

    // --------------------------------------------------------------------------
    // Structs.

    typedef struct TP_Worker
    {
        size_t struct_size;
        void* ext;
        TF_TString task_type;
        TP_Worker_GetTaskTypeFn get_task_type_cb;
        TP_Worker_ExecuteFn execute_cb;
        TP_Worker_ExecuteAsyncFn execute_async_cb;
        TP_Worker_OnErrorFn on_error_cb;
        TP_Worker_OnReleasedFn on_released_cb;
#define TP_WORKER_STRUCT_SIZE TF_OFFSET_OF_END(TP_Worker, on_released_cb)
    } TP_Worker;

    typedef struct TF_WorkerRegistrationParams
    {
        size_t struct_size;
        void* ext;
        int32_t major_version;
        int32_t minor_version;
        int32_t patch_version;
        TP_Worker* worker;
        TF_WorkerRegistrationParams_DestroyWorker destroy_worker;
#define TF_WORKER_REGISTRATION_PARAMS_STRUCT_SIZE                                                  \
    TF_OFFSET_OF_END(TF_WorkerRegistrationParams, destroy_worker)
    } TF_WorkerRegistrationParams;

    // --------------------------------------------------------------------------
    // Registration version.

#define WK_MAJOR 0
#define WK_MINOR 0
#define WK_PATCH 1

    void TF_InitWorker(TF_WorkerRegistrationParams* params, TF_Status* status);

    // --------------------------------------------------------------------------
    // Utility functions.


    static inline TP_Worker* TP_WorkerNew(void)
    {
        TP_Worker* ptr = (TP_Worker*)malloc(TP_WORKER_STRUCT_SIZE);
        if (!ptr) {
            return nullptr;
        }
        ptr->struct_size = TP_WORKER_STRUCT_SIZE;
        ptr->ext = nullptr;
        TF_StringInit(&ptr->task_type);
        ptr->get_task_type_cb = nullptr;
        ptr->execute_cb = nullptr;
        ptr->execute_async_cb = nullptr;
        ptr->on_error_cb = nullptr;
        ptr->on_released_cb = nullptr;
        return ptr;
    }

    static inline void TP_WorkerDelete(TP_Worker* ptr)
    {
        if (!ptr) {
            return;
        }
        TF_StringDealloc(&ptr->task_type);
        free(ptr);
    }

    static inline void TP_Worker_SetTaskType(TP_Worker* builder, TF_TString task_type)
    {
        builder->task_type = task_type;
    }

    static inline void
    TP_Worker_SetGetTaskTypeCallback(TP_Worker* builder, TP_Worker_GetTaskTypeFn get_task_type_cb)
    {
        builder->get_task_type_cb = get_task_type_cb;
    }

    static inline void
    TP_Worker_SetExecuteCallback(TP_Worker* builder, TP_Worker_ExecuteFn execute_cb)
    {
        builder->execute_cb = execute_cb;
    }

    static inline void
    TP_Worker_SetExecuteAsyncCallback(TP_Worker* builder, TP_Worker_ExecuteAsyncFn execute_async_cb)
    {
        builder->execute_async_cb = execute_async_cb;
    }

    static inline void
    TP_Worker_SetOnErrorCallback(TP_Worker* builder, TP_Worker_OnErrorFn on_error_cb)
    {
        builder->on_error_cb = on_error_cb;
    }

    static inline void
    TP_Worker_SetOnReleasedCallback(TP_Worker* builder, TP_Worker_OnReleasedFn on_released_cb)
    {
        builder->on_released_cb = on_released_cb;
    }


#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_WORKER_H_
