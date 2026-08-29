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
#ifndef CONGELADO_C_PROFILER_CONTROLLER_H_
#define CONGELADO_C_PROFILER_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif


    // buffer == NULL means "just report the required size in *size_in_bytes".


    typedef struct TF_Profiler
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_device_type)(void* plugin_context, TF_String* out);
        void (*start)(void* plugin_context, TF_Status* status);
        void (*stop)(void* plugin_context, TF_Status* status);
        void (*collect_data_xspace)(
            void* plugin_context,
            uint8_t* buffer,
            size_t* size_in_bytes,
            TF_Status* status
        );
    } TF_Profiler;

#define TF_PROFILER_STRUCT_SIZE TF_OFFSET_OF_END(TF_Profiler, collect_data_xspace)

    TF_CAPI_EXPORT void init_profiler(TF_Profiler** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_PROFILER_CONTROLLER_H_
