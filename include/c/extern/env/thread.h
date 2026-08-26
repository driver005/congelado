/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

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
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Thread TF_Thread;

    typedef struct TF_ThreadOptions
    {
        size_t struct_size; // Size of this struct (for versioning)
        void* ext;          // Reserved for future extensions
        // Thread stack size to use (in bytes), zero implies that the system default
        // will be used.
        size_t stack_size;

        // Guard area size to use near thread stacks to use (in bytes), zero implies
        // that the system default will be used.
        size_t guard_size;

        // The NUMA node to use, -1 implies that there should be no NUMA affinity for
        // this thread.
        int numa_node;
    } TF_ThreadOptions;

#define TP_THREAD_OPTIONS_STRUCT_SIZE TF_OFFSET_OF_END(TF_ThreadOptions, numa_node)

    // Populates a TF_ThreadOptions struct with system-default values.
    TF_CAPI_EXPORT extern void TF_DefaultThreadOptions(TF_ThreadOptions* options);

    typedef void (*TF_ThreadWorkFn)(void* param);

    // Returns a new thread that is running work_func and is identified
    // (for debugging/performance-analysis) by thread_name.
    //
    // The given param (which may be null) is passed to work_func when the thread
    // starts. In this way, data may be passed from the thread back to the caller.
    //
    // Caller takes ownership of the result and must call TF_JoinThread on it
    // eventually.
    TF_CAPI_EXPORT extern TF_Thread* TF_StartThread(
        const TF_ThreadOptions* options,
        const char* thread_name,
        TF_ThreadWorkFn work_func,
        void* param
    );

    // Waits for the given thread to finish execution, then deletes it.
    TF_CAPI_EXPORT extern void TF_JoinThread(TF_Thread* thread);

    static inline TF_ThreadOptions* TF_ThreadOptionsNew(void)
    {
        TF_ThreadOptions* ptr = (TF_ThreadOptions*)malloc(TP_THREAD_OPTIONS_STRUCT_SIZE);
        if (!ptr) {
            return nullptr;
        }
        ptr->struct_size = TP_THREAD_OPTIONS_STRUCT_SIZE;
        ptr->ext = nullptr;
        ptr->stack_size = 0;
        ptr->guard_size = 0;
        ptr->numa_node = -1;
        return ptr;
    }

    static inline void TF_ThreadOptionsDelete(TF_ThreadOptions* ptr)
    {
        if (!ptr) {
            return;
        }
        free(ptr);
    }

    static inline void TF_ThreadOptions_SetStructSize(TF_ThreadOptions* builder, size_t struct_size)
    {
        builder->struct_size = struct_size;
    }

    static inline void TF_ThreadOptions_SetExt(TF_ThreadOptions* builder, void* ext)
    {
        builder->ext = ext;
    }

    static inline void TF_ThreadOptions_SetStackSize(TF_ThreadOptions* builder, size_t stack_size)
    {
        builder->stack_size = stack_size;
    }

    static inline void TF_ThreadOptions_SetGuardSize(TF_ThreadOptions* builder, size_t guard_size)
    {
        builder->guard_size = guard_size;
    }

    static inline void TF_ThreadOptions_SetNumaNode(TF_ThreadOptions* builder, int numa_node)
    {
        builder->numa_node = numa_node;
    }

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_ENV_THREAD_H_
