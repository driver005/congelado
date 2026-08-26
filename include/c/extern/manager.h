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
#ifndef CONGELADO_C_MANAGER_H_
#define CONGELADO_C_MANAGER_H_

#include "c/abi/macros.h"
#include "c/extern/worker.h"
#include "c/intern/tf_bool.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
typedef struct TP_WorkerManager TP_WorkerManager;

#ifdef __cplusplus
extern "C"
{
#endif

    typedef TF_Bool (*TP_WorkerManager_RequiredFn)(void* user_data);
    typedef void (*TP_WorkerManager_AddWorkerFn)(void* user_data, TP_Worker* worker);
    typedef void (*TP_WorkerManager_SpawnFn)(
        void* user_data, const TF_TString* spec_json, TF_Status* status
    );
    typedef TF_Bool (*TP_WorkerManager_StartFn)(void* user_data, const TF_TString* worker_id);
    typedef TF_Bool (*TP_WorkerManager_StopFn)(void* user_data, const TF_TString* worker_id);
    typedef void (*TP_WorkerManager_ListFn)(void* user_data, TF_TString** out_list_json);

    typedef void (*TF_WorkerManagerRegistrationParams_DestroyManager)(TP_WorkerManager* mgr);

    typedef struct TP_WorkerManager
    {
        size_t struct_size;
        void* ext;
        TF_TString backend_name;
        TP_WorkerManager_RequiredFn required_cb;
        TP_WorkerManager_AddWorkerFn add_worker_cb;
        TP_WorkerManager_SpawnFn spawn_cb;
        TP_WorkerManager_StartFn start_cb;
        TP_WorkerManager_StopFn stop_cb;
        TP_WorkerManager_ListFn list_cb;
#define TP_WORKER_MANAGER_STRUCT_SIZE TF_OFFSET_OF_END(TP_WorkerManager, list_cb)
    } TP_WorkerManager;

    typedef struct TF_WorkerManagerRegistrationParams
    {
        size_t struct_size;
        void* ext;
        int32_t major_version;
        int32_t minor_version;
        int32_t patch_version;
        TP_WorkerManager* manager;
        TF_WorkerManagerRegistrationParams_DestroyManager destroy_manager;
#define TF_WORKER_MANAGER_REGISTRATION_PARAMS_STRUCT_SIZE                                          \
    TF_OFFSET_OF_END(TF_WorkerManagerRegistrationParams, destroy_manager)
    } TF_WorkerManagerRegistrationParams;

#define WM_MAJOR 0
#define WM_MINOR 0
#define WM_PATCH 1

    void TF_InitWorkerManager(TF_WorkerManagerRegistrationParams* params, TF_Status* status);

    static inline TP_WorkerManager* TP_WorkerManagerNew(void)
    {
        TP_WorkerManager* ptr = (TP_WorkerManager*)malloc(TP_WORKER_MANAGER_STRUCT_SIZE);
        if (!ptr) {
            return nullptr;
        }
        ptr->struct_size = TP_WORKER_MANAGER_STRUCT_SIZE;
        ptr->ext = nullptr;
        TF_StringInit(&ptr->backend_name);
        ptr->required_cb = nullptr;
        ptr->add_worker_cb = nullptr;
        ptr->spawn_cb = nullptr;
        ptr->start_cb = nullptr;
        ptr->stop_cb = nullptr;
        ptr->list_cb = nullptr;
        return ptr;
    }

    static inline void TP_WorkerManagerDelete(TP_WorkerManager* ptr)
    {
        if (!ptr) {
            return;
        }
        TF_StringDealloc(&ptr->backend_name);
        free(ptr);
    }

    static inline void
    TP_WorkerManager_SetBackendName(TP_WorkerManager* builder, TF_TString backend_name)
    {
        builder->backend_name = backend_name;
    }

    static inline void TP_WorkerManager_SetRequiredCallback(
        TP_WorkerManager* builder, TP_WorkerManager_RequiredFn required_cb
    )
    {
        builder->required_cb = required_cb;
    }

    static inline void TP_WorkerManager_SetAddWorkerCallback(
        TP_WorkerManager* builder, TP_WorkerManager_AddWorkerFn add_worker_cb
    )
    {
        builder->add_worker_cb = add_worker_cb;
    }

    static inline void
    TP_WorkerManager_SetSpawnCallback(TP_WorkerManager* builder, TP_WorkerManager_SpawnFn spawn_cb)
    {
        builder->spawn_cb = spawn_cb;
    }

    static inline void
    TP_WorkerManager_SetStartCallback(TP_WorkerManager* builder, TP_WorkerManager_StartFn start_cb)
    {
        builder->start_cb = start_cb;
    }

    static inline void
    TP_WorkerManager_SetStopCallback(TP_WorkerManager* builder, TP_WorkerManager_StopFn stop_cb)
    {
        builder->stop_cb = stop_cb;
    }

    static inline void
    TP_WorkerManager_SetListCallback(TP_WorkerManager* builder, TP_WorkerManager_ListFn list_cb)
    {
        builder->list_cb = list_cb;
    }


#ifdef __cplusplus
}
#endif

#endif
