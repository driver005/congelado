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
#ifndef CONGELADO_C_ENV_THREAD_H_
#define CONGELADO_C_ENV_THREAD_H_

#include "c/abi/macros.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Thread TF_Thread;

    typedef void (*TF_ThreadWorkFn)(void* arg);

    typedef struct TF_ThreadOptions
    {
        size_t struct_size;
        int global_name; // If non-zero, set the thread name visible in /proc
    } TF_ThreadOptions;

    TF_CAPI_EXPORT void default_thread_options(TF_ThreadOptions* options);

    // Starts a new thread. The thread name is advisory, used only for debugging.
    // The caller does NOT own the returned TF_Thread*; ownership passes to the
    // returned handle and must be released by calling join_thread.
    TF_CAPI_EXPORT TF_Thread* start_thread(
        const TF_ThreadOptions* options,
        const char* thread_name,
        TF_ThreadWorkFn work_func,
        void* param
    );

    // Blocks until the thread completes, then releases the TF_Thread* handle.
    // Must be called exactly once per start_thread return value.
    TF_CAPI_EXPORT void join_thread(TF_Thread* thread);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_ENV_THREAD_H_
