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
#ifndef CONGELADO_C_MANAGER_CONTROLLER_H_
#define CONGELADO_C_MANAGER_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // No TF_WorkerManager_AddWorker — add_worker takes ownership of a C++ Worker object, which
    // can't cross this C ABI; unsupported for cross-plugin backends (see
    // ice::sonic::WorkerManager::add_worker).


    // out_list_json — caller-supplied TF_String, filled in by the plugin (no free needed).


    // --------------------------------------------------------------------------
    // Error channel: TF_Status* is the sole error channel for every fallible slot.
    // For value-returning slots (bool, int64_t, ...) the returned value is the
    // query result and is meaningful only when status is OK; on failure the value
    // is unspecified and the error lives in *status. Slots with no TF_Status*
    // parameter are pure accessors that cannot fail.
    // --------------------------------------------------------------------------

    typedef struct TF_WorkerManager
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        void (*spawn)(void* plugin_context, const TF_TString* spec_json, TF_Status* status);
        bool (*start)(void* plugin_context, const TF_TString* worker_id, TF_Status* status);
        bool (*stop)(void* plugin_context, const TF_TString* worker_id, TF_Status* status);
        void (*list)(void* plugin_context, TF_String* out_list_json, TF_Status* status);
    } TF_WorkerManager;

#define TF_WORKER_MANAGER_STRUCT_SIZE TF_OFFSET_OF_END(TF_WorkerManager, list)

    TF_CAPI_EXPORT void
    init_worker_manager(TF_WorkerManager** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_MANAGER_CONTROLLER_H_
