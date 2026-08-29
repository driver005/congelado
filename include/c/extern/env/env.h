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

#ifndef CONGELADO_C_ENV_OPS_H_
#define CONGELADO_C_ENV_OPS_H_

#include "c/abi/macros.h"
#include "c/extern/env/dynamic_library.h"
#include "c/extern/env/thread.h"
#include "c/intern/tf_file_statistics.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Env
    {
        size_t struct_size; // Size of this struct (for versioning)
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);

        // dynamic_library
        void* (*load_shared_library)(
            void* plugin_context,
            const TF_TString* library_filename,
            TF_Status* status
        );
        void* (*get_symbol_from_library)(
            void* plugin_context,
            void* handle,
            const TF_TString* symbol_name,
            TF_Status* status
        );


        // thread
        void (*default_thread_options)(void* plugin_context, TF_ThreadOptions* options);
        TF_Thread* (*start_thread)(
            void* plugin_context,
            const TF_ThreadOptions* options,
            const TF_TString* thread_name,
            TF_ThreadWorkFn work_func,
            void* param
        );
        void (*join_thread)(void* plugin_context, TF_Thread* thread);

        // time
        uint64_t (*now_nanos)(void* plugin_context);
        uint64_t (*now_micros)(void* plugin_context);
        uint64_t (*now_seconds)(void* plugin_context);
    } TF_Env;

#define TF_ENV_STRUCT_SIZE TF_OFFSET_OF_END(TF_Env, now_seconds)

    // Initializes the environment operations.
    TF_CAPI_EXPORT void init_env(TF_Env** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_ENV_OPS_H_
