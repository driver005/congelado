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

    // Opaque small-string-optimized string storage. Owned and laid out entirely by whichever backend's create_string() supplied the TF_String ops below — callers never look inside it, only ever hold/pass a pointer.

    typedef struct TF_String
    {
        void* plugin_data;
    } TF_String;

    // Ops vtable for TF_String — matches the intern/extern convention (struct_size first) instead of free functions, so this type registers with cc_abi_gen exactly like TF_Buffer/TF_Shape/TF_Registration.
    typedef struct TF_StringOps
    {
        size_t struct_size;

        void (*init)(TF_String* t);
        void (*copy)(TF_String* dst, const char* src, size_t size);
        void (*assign_view)(TF_String* dst, const char* src, size_t size);
        void (*get_data_pointer)(const TF_String* t, const char** out_data);
        void (*get_type)(const TF_String* t, TF_TString_Type* out_type);
        void (*get_size)(const TF_String* t, size_t* out_size);
        void (*get_capacity)(const TF_String* t, size_t* out_capacity);
        void (*dealloc)(TF_String* t);
    } TF_StringOps;

#define TF_STRING_STRUCT_SIZE TF_OFFSET_OF_END(TF_StringOps, dealloc)

    // Declared-only, like create_buffer/create_shape: no default implementation lives anywhere in the repo, graceful null-ops degradation expected.
    TF_CAPI_EXPORT void create_string(TF_StringOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_string(void* plugin_context);

    // Real implementation, not declared-only — calls create_string
    static inline void init_string(TF_String* t, TF_Status* out_status)
    {
        TF_StringOps* ops = NULL;
        create_string(&ops, &t->plugin_data, out_status);
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_TSTRING_H_
