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
#ifndef CONGELADO_C_CRON_H_
#define CONGELADO_C_CRON_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "c/abi/macros.h"
#include "c/intern/tf_bool.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

// --------------------------------------------------------------------------
// C API for Cron. The API allows registering a pluggable cron scheduler with
// Congelado.
//
// Conventions:
//   * TF_: Set/filled by core, unless marked otherwise.
//   * TP_: Set/filled by plug-in, unless marked otherwise.
//   * Structs begin with size_t struct_size and void* ext.

#ifdef __cplusplus
extern "C" {
#endif

// Callback invoked when a cron job fires.
typedef void (*TF_Cron_FireFn)(const TF_TString* job_name, void* user_data);

// --------------------------------------------------------------------------
// Cron callback types — defined separately for use in struct and C++ module.
typedef TF_Bool (*TP_Cron_RequiredFn)(void* user_data);
typedef TF_Bool (*TP_Cron_ValidateFn)(void* user_data, const TF_TString* expression);
typedef TF_Bool (*TP_Cron_NextAfterFn)(void* user_data, const TF_TString* expression,
                                       int64_t base_time_ms, int64_t* out_time_ms);
typedef void (*TP_Cron_UpsertJobFn)(void* user_data, const TF_TString* name,
                                     const TF_TString* expression);
typedef void (*TP_Cron_RemoveJobFn)(void* user_data, const TF_TString* name);

// Forward declaration — TP_Cron is fully defined below; needed here for
// TF_CronRegistrationParams_DestroyCron which references TP_Cron*.
typedef struct TP_Cron TP_Cron;

typedef void (*TF_CronRegistrationParams_DestroyCron)(TP_Cron* cron);

// --------------------------------------------------------------------------
// TP_Cron holds cron configuration and user callbacks.
typedef struct TP_Cron {
  size_t struct_size;
  void* ext;  // free-form data set by plugin.

  // Backend name (small-string-optimized).
  TF_TString backend_name;

  // User callbacks.
  TP_Cron_RequiredFn required_cb;
  TP_Cron_ValidateFn validate_cb;
  TP_Cron_NextAfterFn next_after_cb;
  TP_Cron_UpsertJobFn upsert_job_cb;
  TP_Cron_RemoveJobFn remove_job_cb;

  // The struct size must be updated when adding new members.
#define TP_CRON_STRUCT_SIZE TF_OFFSET_OF_END(TP_Cron, remove_job_cb)
} TP_Cron;

// --------------------------------------------------------------------------
// TF_CronRegistrationParams holds the pointers to TP_Cron.
typedef struct TF_CronRegistrationParams {
  size_t struct_size;
  void* ext;  // reserved for future use

  // Cron C API version.
  int32_t major_version;
  int32_t minor_version;
  int32_t patch_version;

  // [in/out] Memory owned by core but attributes within are populated by the
  // plugin.
  TP_Cron* cron;

  // [out] Pointer to plugin's cleanup function.
  TF_CronRegistrationParams_DestroyCron destroy_cron;

  // The struct size must be updated when adding new members.
#define TF_CRON_REGISTRATION_PARAMS_STRUCT_SIZE \
  TF_OFFSET_OF_END(TF_CronRegistrationParams, destroy_cron)
} TF_CronRegistrationParams;

#define CR_MAJOR 0
#define CR_MINOR 0
#define CR_PATCH 1

// TF_InitCron is used to do cron registration.
void TF_InitCron(TF_CronRegistrationParams* params, TF_Status* status);

// TP_CronAlloc — allocates and initializes a TP_Cron struct.


static inline TP_Cron* TP_CronNew(void) {
  TP_Cron* ptr = (TP_Cron*)malloc(TP_CRON_STRUCT_SIZE);
  if (!ptr) return nullptr;
  ptr->struct_size = TP_CRON_STRUCT_SIZE;
  ptr->ext = nullptr;
  TF_StringInit(&ptr->backend_name);
  ptr->required_cb = nullptr;
  ptr->validate_cb = nullptr;
  ptr->next_after_cb = nullptr;
  ptr->upsert_job_cb = nullptr;
  ptr->remove_job_cb = nullptr;
  return ptr;

}

// TP_CronDealloc — frees a TP_Cron allocated by TP_CronAlloc.
static inline void TP_CronDelete(TP_Cron* ptr) {
  if (!ptr) return;
  TF_StringDealloc(&ptr->backend_name);
  free(ptr);
}

static inline void TP_Cron_SetRequiredCallback(TP_Cron* builder, TP_Cron_RequiredFn required_cb) {
  builder->required_cb = required_cb;
}

static inline void TP_Cron_SetValidateCallback(TP_Cron* builder, TP_Cron_ValidateFn validate_cb) {
  builder->validate_cb = validate_cb;
}

static inline void TP_Cron_SetNextAfterCallback(TP_Cron* builder, TP_Cron_NextAfterFn next_after_cb) {
  builder->next_after_cb = next_after_cb;
}

static inline void TP_Cron_SetUpsertJobCallback(TP_Cron* builder, TP_Cron_UpsertJobFn upsert_job_cb) {
  builder->upsert_job_cb = upsert_job_cb;
}

static inline void TP_Cron_SetRemoveJobCallback(TP_Cron* builder, TP_Cron_RemoveJobFn remove_job_cb) {
  builder->remove_job_cb = remove_job_cb;
}



#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_CRON_H_
