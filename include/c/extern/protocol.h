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
#ifndef TENSORFLOW_C_PROTOCOL_PROTOCOL_H_
#define TENSORFLOW_C_PROTOCOL_PROTOCOL_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "c/abi/macros.h"
#include "c/intern/tf_bool.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
typedef struct TP_Protocol_Server TP_Protocol_Server;
typedef struct TP_Protocol TP_Protocol;

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------
// Callback types

struct TP_Protocol_Server;
struct TP_Protocol;

typedef void (*TP_Protocol_Server_StartFn)(void* user_data, TF_Status* status);
typedef void (*TP_Protocol_Server_StopFn)(void* user_data, TF_Status* status);
typedef TF_Bool (*TP_Protocol_Server_IsRunningFn)(void* user_data);

typedef TP_Protocol_Server* (*TP_Protocol_CreateServerFn)(void* user_data,
                                                           TF_Status* status);

typedef void (*TF_ProtocolRegistrationParams_DestroyProtocol)(TP_Protocol* protocol);
typedef void (*TF_ProtocolRegistrationParams_DestroyServer)(TP_Protocol_Server* server);

// --------------------------------------------------------------------------
// Structs

typedef struct TP_Protocol_Server {
  size_t struct_size;
  void* ext;
  TP_Protocol_Server_StartFn start_cb;
  TP_Protocol_Server_StopFn stop_cb;
  TP_Protocol_Server_IsRunningFn is_running_cb;
#define TP_PROTOCOL_SERVER_STRUCT_SIZE \
  TF_OFFSET_OF_END(TP_Protocol_Server, is_running_cb)
} TP_Protocol_Server;

typedef struct TP_Protocol {
  size_t struct_size;
  void* ext;
  TF_TString name;
  TF_TString bind_host;
  uint16_t bind_port;
  TF_TString tls_cert;
  TF_TString tls_key;
  TP_Protocol_CreateServerFn create_server_cb;
#define TP_PROTOCOL_STRUCT_SIZE TF_OFFSET_OF_END(TP_Protocol, create_server_cb)
} TP_Protocol;

typedef struct TF_ProtocolRegistrationParams {
  size_t struct_size;
  void* ext;
  int32_t major_version;
  int32_t minor_version;
  int32_t patch_version;
  TP_Protocol* protocol;
  TF_ProtocolRegistrationParams_DestroyProtocol destroy_protocol;
  TF_ProtocolRegistrationParams_DestroyServer destroy_server;
#define TF_PROTOCOL_REGISTRATION_PARAMS_STRUCT_SIZE \
  TF_OFFSET_OF_END(TF_ProtocolRegistrationParams, destroy_server)
} TF_ProtocolRegistrationParams;

// --------------------------------------------------------------------------
// Registration

#define PR_MAJOR 0
#define PR_MINOR 0
#define PR_PATCH 1

void TF_InitProtocol(TF_ProtocolRegistrationParams* params, TF_Status* status);

// --------------------------------------------------------------------------
// Utility inlines



static inline TP_Protocol* TP_ProtocolNew(void) {
  TP_Protocol* ptr = (TP_Protocol*)malloc(TP_PROTOCOL_STRUCT_SIZE);
  if (!ptr) return nullptr;
  ptr->struct_size = TP_PROTOCOL_STRUCT_SIZE;
  ptr->ext = nullptr;
  TF_StringInit(&ptr->name);
  TF_StringInit(&ptr->bind_host);
  TF_StringInit(&ptr->tls_cert);
  TF_StringInit(&ptr->tls_key);
  ptr->create_server_cb = nullptr;
  return ptr;

}

static inline void TP_ProtocolDelete(TP_Protocol* ptr) {
  if (!ptr) return;
  TF_StringDealloc(&ptr->name);
  TF_StringDealloc(&ptr->bind_host);
  TF_StringDealloc(&ptr->tls_cert);
  TF_StringDealloc(&ptr->tls_key);
  free(ptr);
}

static inline void TP_Protocol_SetName(TP_Protocol* builder, TF_TString name) {
  builder->name = name;
}

static inline void TP_Protocol_SetBindHost(TP_Protocol* builder, TF_TString bind_host) {
  builder->bind_host = bind_host;
}

static inline void TP_Protocol_SetBindPort(TP_Protocol* builder, uint16_t bind_port) {
  builder->bind_port = bind_port;
}

static inline void TP_Protocol_SetTlsCert(TP_Protocol* builder, TF_TString tls_cert) {
  builder->tls_cert = tls_cert;
}

static inline void TP_Protocol_SetTlsKey(TP_Protocol* builder, TF_TString tls_key) {
  builder->tls_key = tls_key;
}

static inline void TP_Protocol_SetCreateServerCallback(TP_Protocol* builder, TP_Protocol_CreateServerFn create_server_cb) {
  builder->create_server_cb = create_server_cb;
}



#ifdef __cplusplus
}
#endif

#endif
