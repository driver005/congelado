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

#ifndef CONGELADO_C_OTEL_COUNTER_H_
#define CONGELADO_C_OTEL_COUNTER_H_

#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*TP_Otel_Counter_AddFn)(void* user_data, double value);

typedef struct TP_Otel_Counter {
  size_t struct_size;
  void* ext;
  TP_Otel_Counter_AddFn add_cb;
#define TP_OTEL_COUNTER_STRUCT_SIZE TF_OFFSET_OF_END(TP_Otel_Counter, add_cb)
} TP_Otel_Counter;

// LINT.ThenChange(:otel_counter_version)



static inline TP_Otel_Counter* TP_OtelCounterNew(void) {
  TP_Otel_Counter* ptr = (TP_Otel_Counter*)malloc(sizeof(struct TP_Otel_Counter));
  if (!ptr) return nullptr;
  ptr->struct_size = sizeof(struct TP_Otel_Counter);
  ptr->ext = nullptr;
  ptr->add_cb = nullptr;
  return ptr;
}

static inline void TP_OtelCounterDelete(TP_Otel_Counter* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TP_OtelCounter_SetStructSize(TP_Otel_Counter* builder, size_t struct_size) {
  builder->struct_size = struct_size;
}

static inline void TP_OtelCounter_SetExt(TP_Otel_Counter* builder, void* ext) {
  builder->ext = ext;
}

static inline void TP_OtelCounter_SetAddCallback(TP_Otel_Counter* builder, TP_Otel_Counter_AddFn add_cb) {
  builder->add_cb = add_cb;
}



#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_OTEL_COUNTER_H_