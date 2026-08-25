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
#ifndef CONGELADO_C_PAYLOAD_H_
#define CONGELADO_C_PAYLOAD_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "c/abi/macros.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum TF_Payload_Type {
  TF_PAYLOAD_WORKFLOW_INPUT = 0,
  TF_PAYLOAD_WORKFLOW_OUTPUT = 1,
  TF_PAYLOAD_TASK_INPUT = 2,
  TF_PAYLOAD_TASK_OUTPUT = 3,
} TF_Payload_Type;

typedef void (*TF_Payload_CompletionFn)(const TF_TString* result, void* user_data);

typedef void (*TP_Payload_WriteFn)(void* user_data, TF_Payload_Type type,
                                   const TF_TString* data,
                                   TF_Payload_CompletionFn completion,
                                   void* cb_user_data);
typedef void (*TP_Payload_ReadFn)(void* user_data, const TF_TString* reference,
                                  TF_Payload_CompletionFn completion,
                                  void* cb_user_data);

// Forward declaration — TP_Payload is fully defined below; needed here for
// TF_PayloadRegistrationParams_DestroyPayload which references TP_Payload*.
typedef struct TP_Payload TP_Payload;

typedef void (*TF_PayloadRegistrationParams_DestroyPayload)(TP_Payload* payload);

typedef struct TP_Payload {
  size_t struct_size;
  void* ext;
  TP_Payload_WriteFn write_cb;
  TP_Payload_ReadFn read_cb;
#define TP_PAYLOAD_STRUCT_SIZE TF_OFFSET_OF_END(TP_Payload, read_cb)
} TP_Payload;

typedef struct TF_PayloadRegistrationParams {
  size_t struct_size;
  void* ext;
  int32_t major_version;
  int32_t minor_version;
  int32_t patch_version;
  TP_Payload* payload;
  TF_PayloadRegistrationParams_DestroyPayload destroy_payload;
#define TF_PAYLOAD_REGISTRATION_PARAMS_STRUCT_SIZE \
  TF_OFFSET_OF_END(TF_PayloadRegistrationParams, destroy_payload)
} TF_PayloadRegistrationParams;

#define PL_MAJOR 0
#define PL_MINOR 0
#define PL_PATCH 1

void TF_InitPayload(TF_PayloadRegistrationParams* params, TF_Status* status);



static inline TP_Payload* TP_PayloadNew(void) {
  TP_Payload* ptr = (TP_Payload*)malloc(TP_PAYLOAD_STRUCT_SIZE);
  if (!ptr) return nullptr;
  ptr->struct_size = TP_PAYLOAD_STRUCT_SIZE;
  ptr->ext = nullptr;
  ptr->write_cb = nullptr;
  ptr->read_cb = nullptr;
  return ptr;

}

static inline void TP_PayloadDelete(TP_Payload* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TP_Payload_SetWriteCallback(TP_Payload* builder, TP_Payload_WriteFn write_cb) {
  builder->write_cb = write_cb;
}

static inline void TP_Payload_SetReadCallback(TP_Payload* builder, TP_Payload_ReadFn read_cb) {
  builder->read_cb = read_cb;
}



#ifdef __cplusplus
}
#endif

#endif
