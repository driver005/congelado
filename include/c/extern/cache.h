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
#ifndef CONGELADO_C_CACHE_H_
#define CONGELADO_C_CACHE_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "c/abi/macros.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_status.h"

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------
// C API for Cache. The API allows registering a pluggable cache backend with
// Congelado.
//
// Conventions:
//   * TF_: Set/filled by core, unless marked otherwise.
//   * TP_: Set/filled by plug-in, unless marked otherwise.
//   * Structs begin with size_t struct_size and void* ext.
//   * TF_CAPI_EXPORT on API functions, not on TF_InitXxx.

// --------------------------------------------------------------------------
// Callback types.

// Callback for async cache operations.
// result: null-terminated string result, or NULL if not found.
// user_data: opaque pointer passed through from the caller.
typedef void (*TF_Cache_CompletionFn)(const TF_TString* result, void* user_data);

// --------------------------------------------------------------------------
// Cache callback types — defined separately for use in struct and C++ module.
typedef void (*TP_Cache_GetFn)(void* user_data, const TF_TString* key,
                               TF_Cache_CompletionFn completion, void* cb_user_data);
typedef void (*TP_Cache_SetFn)(void* user_data, const TF_TString* key,
                               const TF_TString* value,
                               TF_Cache_CompletionFn completion, void* cb_user_data);
typedef void (*TP_Cache_RemoveFn)(void* user_data, const TF_TString* key,
                                   TF_Cache_CompletionFn completion, void* cb_user_data);

// Forward declaration — TP_Cache is fully defined below; needed here for
// TF_CacheRegistrationParams_DestroyCache which references TP_Cache*.
typedef struct TP_Cache TP_Cache;

typedef void (*TF_CacheRegistrationParams_DestroyCache)(TP_Cache* cache);

// --------------------------------------------------------------------------
// TP_Cache holds cache configuration and user callbacks.
typedef struct TP_Cache {
  size_t struct_size;
  void* ext;  // free-form data set by plugin.

  // Backend name (small-string-optimized).
  TF_TString backend_name;

  // User callbacks.
  TP_Cache_GetFn get_cb;
  TP_Cache_SetFn set_cb;
  TP_Cache_RemoveFn remove_cb;

  // The struct size must be updated when adding new members.
#define TP_CACHE_STRUCT_SIZE TF_OFFSET_OF_END(TP_Cache, remove_cb)
} TP_Cache;

// --------------------------------------------------------------------------
// TF_CacheRegistrationParams holds the pointers to TP_Cache.
typedef struct TF_CacheRegistrationParams {
  size_t struct_size;
  void* ext;  // reserved for future use

  // Cache C API version.
  int32_t major_version;
  int32_t minor_version;
  int32_t patch_version;

  // [in/out] Memory owned by core but attributes within are populated by the
  // plugin.
  TP_Cache* cache;

  // [out] Pointer to plugin's cleanup function.
  TF_CacheRegistrationParams_DestroyCache destroy_cache;

  // The struct size must be updated when adding new members.
#define TF_CACHE_REGISTRATION_PARAMS_STRUCT_SIZE \
  TF_OFFSET_OF_END(TF_CacheRegistrationParams, destroy_cache)
} TF_CacheRegistrationParams;

// --------------------------------------------------------------------------
// Registration version.

#define CA_MAJOR 0
#define CA_MINOR 0
#define CA_PATCH 1

// TF_InitCache is used to do cache registration.
// Plugin should implement TF_InitCache to register the cache backend.
void TF_InitCache(TF_CacheRegistrationParams* params, TF_Status* status);

// --------------------------------------------------------------------------
// Utility functions.

// TP_CacheAlloc — allocates and initializes a TP_Cache struct.


static inline TP_Cache* TP_CacheNew(void) {
  TP_Cache* ptr = (TP_Cache*)malloc(TP_CACHE_STRUCT_SIZE);
  if (!ptr) return nullptr;
  ptr->struct_size = TP_CACHE_STRUCT_SIZE;
  ptr->ext = nullptr;
  TF_StringInit(&ptr->backend_name);
  ptr->get_cb = nullptr;
  ptr->set_cb = nullptr;
  ptr->remove_cb = nullptr;
  return ptr;

}

// TP_CacheDealloc — frees a TP_Cache.
static inline void TP_CacheDelete(TP_Cache* ptr) {
  if (!ptr) return;
  TF_StringDealloc(&ptr->backend_name);
  free(ptr);
}

static inline void TP_Cache_SetGetCallback(TP_Cache* builder, TP_Cache_GetFn get_cb) {
  builder->get_cb = get_cb;
}

static inline void TP_Cache_SetSetCallback(TP_Cache* builder, TP_Cache_SetFn set_cb) {
  builder->set_cb = set_cb;
}

static inline void TP_Cache_SetRemoveCallback(TP_Cache* builder, TP_Cache_RemoveFn remove_cb) {
  builder->remove_cb = remove_cb;
}



#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_CACHE_H_
