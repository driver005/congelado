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
#ifndef CONGELADO_C_SERDE_H_
#define CONGELADO_C_SERDE_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
typedef struct TP_Serde TP_Serde;

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------
// Callback types.

typedef TF_Status* (*TP_Serde_EncodeFn)(void* user_data,
    const TF_TString* value_json, TF_TString** out_encoded);
typedef TF_Status* (*TP_Serde_DecodeFn)(void* user_data,
    const TF_TString* data, TF_TString** out_json);

typedef void (*TF_SerdeRegistrationParams_DestroySerde)(TP_Serde* serde);

// --------------------------------------------------------------------------
// Structs.

typedef struct TP_Serde {
  size_t struct_size;
  void* ext;
  TF_TString content_type;
  TF_TString format_name;
  TP_Serde_EncodeFn encode_cb;
  TP_Serde_DecodeFn decode_cb;
#define TP_SERDE_STRUCT_SIZE TF_OFFSET_OF_END(TP_Serde, decode_cb)
} TP_Serde;

typedef struct TF_SerdeRegistrationParams {
  size_t struct_size;
  void* ext;
  int32_t major_version;
  int32_t minor_version;
  int32_t patch_version;
  TP_Serde* serde;
  TF_SerdeRegistrationParams_DestroySerde destroy_serde;
#define TF_SERDE_REGISTRATION_PARAMS_STRUCT_SIZE \
  TF_OFFSET_OF_END(TF_SerdeRegistrationParams, destroy_serde)
} TF_SerdeRegistrationParams;

// --------------------------------------------------------------------------
// Registration version.

#define SE_MAJOR 0
#define SE_MINOR 0
#define SE_PATCH 1

void TF_InitSerde(TF_SerdeRegistrationParams* params, TF_Status* status);
TF_CAPI_EXPORT extern void TF_Serde_FreeString(TF_TString* str);

// --------------------------------------------------------------------------
// Utility functions.



static inline TP_Serde* TP_SerdeNew(void) {
  TP_Serde* ptr = (TP_Serde*)malloc(TP_SERDE_STRUCT_SIZE);
  if (!ptr) return nullptr;
  ptr->struct_size = TP_SERDE_STRUCT_SIZE;
  ptr->ext = nullptr;
  TF_StringInit(&ptr->content_type);
  TF_StringInit(&ptr->format_name);
  ptr->encode_cb = nullptr;
  ptr->decode_cb = nullptr;
  return ptr;

}

static inline void TP_SerdeDelete(TP_Serde* ptr) {
  if (!ptr) return;
  TF_StringDealloc(&ptr->content_type);
  TF_StringDealloc(&ptr->format_name);
  free(ptr);
}

static inline void TP_Serde_SetContentType(TP_Serde* builder, TF_TString content_type) {
  builder->content_type = content_type;
}

static inline void TP_Serde_SetFormatName(TP_Serde* builder, TF_TString format_name) {
  builder->format_name = format_name;
}

static inline void TP_Serde_SetEncodeCallback(TP_Serde* builder, TP_Serde_EncodeFn encode_cb) {
  builder->encode_cb = encode_cb;
}

static inline void TP_Serde_SetDecodeCallback(TP_Serde* builder, TP_Serde_DecodeFn decode_cb) {
  builder->decode_cb = decode_cb;
}



#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_SERDE_H_
