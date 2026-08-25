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
#ifndef CONGELADO_C_SEARCH_H_
#define CONGELADO_C_SEARCH_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "c/abi/macros.h"
#include "c/intern/tf_bool.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------
// Callback types

typedef void (*TF_Search_CompletionFn)(const TF_TString* result_json,
                                       void* user_data);

typedef void (*TP_Search_IndexFn)(void* user_data, const TF_TString* collection,
                                   const TF_TString* id,
                                   const TF_TString* document_json,
                                   TF_Search_CompletionFn completion,
                                   void* cb_user_data);
typedef void (*TP_Search_RemoveFn)(void* user_data,
                                    const TF_TString* collection,
                                    const TF_TString* id,
                                    TF_Search_CompletionFn completion,
                                    void* cb_user_data);
// Forward declarations — TF_Search_Query and TP_Search are fully defined
// below; needed here for the callback typedefs which reference them.
typedef struct TF_Search_Query TF_Search_Query;
typedef struct TP_Search TP_Search;

typedef void (*TP_Search_SearchFn)(void* user_data,
                                    const TF_TString* collection,
                                    const TF_Search_Query* query,
                                    TF_Search_CompletionFn completion,
                                    void* cb_user_data);

typedef void (*TF_SearchRegistrationParams_DestroySearch)(TP_Search* search);

// --------------------------------------------------------------------------
// Structs

typedef struct TF_Search_Query {
  size_t struct_size;
  void* ext;
  TF_TString query;
  TF_TString free_text;
  int64_t start;
  int64_t size;
  TF_TString sort;
#define TF_SEARCH_QUERY_STRUCT_SIZE TF_OFFSET_OF_END(TF_Search_Query, sort)
} TF_Search_Query;

typedef struct TP_Search {
  size_t struct_size;
  void* ext;
  TF_TString backend_name;
  TP_Search_IndexFn index_cb;
  TP_Search_RemoveFn remove_cb;
  TP_Search_SearchFn search_cb;
#define TP_SEARCH_STRUCT_SIZE TF_OFFSET_OF_END(TP_Search, search_cb)
} TP_Search;

typedef struct TF_SearchRegistrationParams {
  size_t struct_size;
  void* ext;
  int32_t major_version;
  int32_t minor_version;
  int32_t patch_version;
  TP_Search* search;
  TF_SearchRegistrationParams_DestroySearch destroy_search;
#define TF_SEARCH_REGISTRATION_PARAMS_STRUCT_SIZE \
  TF_OFFSET_OF_END(TF_SearchRegistrationParams, destroy_search)
} TF_SearchRegistrationParams;

// --------------------------------------------------------------------------
// Registration

#define SR_MAJOR 0
#define SR_MINOR 0
#define SR_PATCH 1

void TF_InitSearch(TF_SearchRegistrationParams* params, TF_Status* status);

// --------------------------------------------------------------------------
// Utility inlines



static inline TP_Search* TP_SearchNew(void) {
  TP_Search* ptr = (TP_Search*)malloc(TP_SEARCH_STRUCT_SIZE);
  if (!ptr) return nullptr;
  ptr->struct_size = TP_SEARCH_STRUCT_SIZE;
  ptr->ext = nullptr;
  TF_StringInit(&ptr->backend_name);
  ptr->index_cb = nullptr;
  ptr->remove_cb = nullptr;
  ptr->search_cb = nullptr;
  return ptr;

}

static inline void TP_SearchDelete(TP_Search* ptr) {
  if (!ptr) return;
  TF_StringDealloc(&ptr->backend_name);
  free(ptr);
}

static inline void TP_Search_SetBackendName(TP_Search* builder, TF_TString backend_name) {
  builder->backend_name = backend_name;
}

static inline void TP_Search_SetIndexCallback(TP_Search* builder, TP_Search_IndexFn index_cb) {
  builder->index_cb = index_cb;
}

static inline void TP_Search_SetRemoveCallback(TP_Search* builder, TP_Search_RemoveFn remove_cb) {
  builder->remove_cb = remove_cb;
}

static inline void TP_Search_SetSearchCallback(TP_Search* builder, TP_Search_SearchFn search_cb) {
  builder->search_cb = search_cb;
}



#ifdef __cplusplus
}
#endif

#endif
