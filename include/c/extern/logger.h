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
#ifndef CONGELADO_C_LOGGER_LOGGER_H_
#define CONGELADO_C_LOGGER_LOGGER_H_

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

typedef enum TF_Logger_LogLevel {
  TF_LOGGER_DEBUG = 0,
  TF_LOGGER_INFO = 1,
  TF_LOGGER_IMPORTANT = 2,
  TF_LOGGER_WARNING = 3,
  TF_LOGGER_ERROR = 4,
  TF_LOGGER_FATAL = 5,
} TF_Logger_LogLevel;

typedef void (*TP_Logger_WriteFn)(void* user_data, TF_Logger_LogLevel level,
                                  const TF_TString* message);
typedef TF_Bool (*TP_Logger_RequiredFn)(void* user_data);

typedef struct TP_Logger {
  size_t struct_size;
  void* ext;
  TF_TString name;
  TP_Logger_WriteFn write_cb;
  TP_Logger_RequiredFn required_cb;
#define TP_LOGGER_STRUCT_SIZE TF_OFFSET_OF_END(TP_Logger, required_cb)
} TP_Logger;

typedef struct TF_LoggerRegistrationParams {
  size_t struct_size;
  void* ext;
  int32_t major_version;
  int32_t minor_version;
  int32_t patch_version;
  TP_Logger* logger;
} TF_LoggerRegistrationParams;

void TF_InitLogger(TF_LoggerRegistrationParams* params, TF_Status* status);
const char* TF_Logger_LogLevelToString(TF_Logger_LogLevel level);



static inline TP_Logger* TP_LoggerNew(void) {
  TP_Logger* ptr = (TP_Logger*)malloc(TP_LOGGER_STRUCT_SIZE);
  if (!ptr) return nullptr;
  ptr->struct_size = TP_LOGGER_STRUCT_SIZE;
  ptr->ext = nullptr;
  TF_StringInit(&ptr->name);
  ptr->write_cb = nullptr;
  ptr->required_cb = nullptr;
  return ptr;

}

static inline void TP_LoggerDelete(TP_Logger* ptr) {
  if (!ptr) return;
  TF_StringDealloc(&ptr->name);
  free(ptr);
}

static inline void TP_Logger_SetName(TP_Logger* builder, TF_TString name) {
  builder->name = name;
}

static inline void TP_Logger_SetWriteCallback(TP_Logger* builder, TP_Logger_WriteFn write_cb) {
  builder->write_cb = write_cb;
}

static inline void TP_Logger_SetRequiredCallback(TP_Logger* builder, TP_Logger_RequiredFn required_cb) {
  builder->required_cb = required_cb;
}



#ifdef __cplusplus
}
#endif

#endif
