/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

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

#ifndef TENSORFLOW_C_TF_TSTRING_H_
#define TENSORFLOW_C_TF_TSTRING_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_TString types for small-string optimization.
    typedef enum TF_TString_Type
    {
        TF_TSTR_SMALL = 0,
        TF_TSTR_LARGE = 1,
        TF_TSTR_OFFSET = 2,
        TF_TSTR_VIEW = 3
    } TF_TString_Type;

    // Opaque small-string-optimized string storage. Owned and laid out entirely by whichever
    // backend's init_string() supplied the TF_String ops below — callers never look inside it,
    // only ever hold/pass a pointer.
    typedef struct TF_String_Handle TF_String_Handle;

    // Ops vtable for TF_String_Handle — matches the intern/extern convention (struct_size first,
    // every slot takes plugin_context first) instead of free functions, so this type registers
    // with cc_abi_gen exactly like TF_Buffer/TF_Shape/TF_Registration.
    typedef struct TF_String
    {
        size_t struct_size;

        TF_String_Handle* (*new_tstring)(void* plugin_context);
        void (*init)(TF_String_Handle* t);
        void (*copy)(TF_String_Handle* dst, const char* src, size_t size);
        void (*assign_view)(TF_String_Handle* dst, const char* src, size_t size);
        const char* (*get_data_pointer)(const TF_String_Handle* t);
        TF_TString_Type (*get_type)(const TF_String_Handle* t);
        size_t (*get_size)(const TF_String_Handle* t);
        size_t (*get_capacity)(const TF_String_Handle* t);
        void (*dealloc)(TF_String_Handle* t);
    } TF_String;

#define TF_STRING_STRUCT_SIZE TF_OFFSET_OF_END(TF_String, dealloc)

    // Declared-only, like init_buffer/init_shape: no default implementation lives anywhere in
    // the repo, graceful null-ops degradation expected.
    TF_CAPI_EXPORT void init_string(TF_String** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_TSTRING_H_
