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

#ifndef CONGELADO_C_OTEL_TRACER_H_
#define CONGELADO_C_OTEL_TRACER_H_

#include <stddef.h>
#include <stdlib.h>

#include "c/intern/tf_tstring.h"
#include "c/intern/tf_status.h"

#include "c/extern/otel/opaque_types.h"
#include "c/extern/otel/span.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TP_Otel_Span* (*TP_Otel_Tracer_StartSpanFn)(void* user_data,
    const TF_TString* name, int kind, TF_Status* status);

typedef struct TP_Otel_Tracer {
  size_t struct_size;
  void* ext;
  TP_Otel_Tracer_StartSpanFn start_span_cb;
#define TP_OTEL_TRACER_STRUCT_SIZE TF_OFFSET_OF_END(TP_Otel_Tracer, start_span_cb)
} TP_Otel_Tracer;

// LINT.ThenChange(:otel_tracer_version)



static inline TP_Otel_Tracer* TP_OtelTracerNew(void) {
  TP_Otel_Tracer* ptr = (TP_Otel_Tracer*)malloc(sizeof(struct TP_Otel_Tracer));
  if (!ptr) return nullptr;
  ptr->struct_size = sizeof(struct TP_Otel_Tracer);
  ptr->ext = nullptr;
  ptr->start_span_cb = nullptr;
  return ptr;
}

static inline void TP_OtelTracerDelete(TP_Otel_Tracer* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TP_OtelTracer_SetStructSize(TP_Otel_Tracer* builder, size_t struct_size) {
  builder->struct_size = struct_size;
}

static inline void TP_OtelTracer_SetExt(TP_Otel_Tracer* builder, void* ext) {
  builder->ext = ext;
}

static inline void TP_OtelTracer_SetStartSpanCallback(TP_Otel_Tracer* builder, TP_Otel_Tracer_StartSpanFn start_span_cb) {
  builder->start_span_cb = start_span_cb;
}



#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_OTEL_TRACER_H_