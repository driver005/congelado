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
#ifndef CONGELADO_C_SEARCH_CONTROLLER_H_
#define CONGELADO_C_SEARCH_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*TF_Search_CompletionFn)(const TF_TString* result_json, void* user_data);

    // TF_Search_Query — a plain value struct (not a callback vtable), passed by the caller into
    // TF_Search_Search. The mainframe constructs one on the stack; no allocator function exists
    // for it, matching the shape it always had (see ice::builder::SearchQuery, the
    // pure-C++ counterpart the sonic adapter converts to/from this at the C-ABI boundary).
    typedef struct TF_Search_Query
    {
        size_t struct_size;
        void* ext;
        TF_TString query;
        TF_TString free_text;
        int64_t start;
        int64_t size;
        TF_TString sort;
#define TF_SEARCH_QUERY_STRUCT_SIZE TF_OFFSET_OF_END(TF_Search_Query, sort)
    } TF_Search_Query;

    typedef struct TF_Search
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        void (*index)(
            void* plugin_context,
            const TF_TString* collection,
            const TF_TString* id,
            const TF_TString* document_json,
            TF_Search_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
        void (*remove)(
            void* plugin_context,
            const TF_TString* collection,
            const TF_TString* id,
            TF_Search_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
        void (*search)(
            void* plugin_context,
            const TF_TString* collection,
            const TF_Search_Query* query,
            TF_Search_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
    } TF_Search;

#define TF_SEARCH_STRUCT_SIZE TF_OFFSET_OF_END(TF_Search, search)

    TF_CAPI_EXPORT void init_search(TF_Search** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_SEARCH_CONTROLLER_H_
