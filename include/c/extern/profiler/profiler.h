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

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tensor.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Profiler
    {
        void* plugin_data;
    } TF_Profiler;

    typedef struct TF_ProfilerOps
    {
        size_t struct_size;
        void (*destroy)(TF_Profiler* profiler);
        void (*get_name)(TF_Profiler* profiler, TF_String* out_name);
        void (*get_device_type)(TF_Profiler* profiler, TF_String* out_device_type);
        void (*start)(TF_Profiler* profiler, TF_Status* out_status);
        void (*stop)(TF_Profiler* profiler, TF_Status* out_status);
        void (*collect_data_xspace)(TF_Profiler* profiler, TF_Tensor** out_data, TF_Status* out_status);
    } TF_ProfilerOps;

#define TF_PROFILER_STRUCT_SIZE TF_OFFSET_OF_END(TF_ProfilerOps, collect_data_xspace)

    TF_CAPI_EXPORT void create_profiler(TF_ProfilerOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_profiler(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_PROFILER_CONTROLLER_H_
