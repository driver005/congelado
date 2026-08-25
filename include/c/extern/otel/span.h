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

#ifndef CONGELADO_C_OTEL_SPAN_H_
#define CONGELADO_C_OTEL_SPAN_H_

#include <stddef.h>
#include <stdlib.h>

#include "c/intern/tf_tstring.h"
#include "c/intern/tf_status.h"

#include "c/extern/otel/opaque_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*TP_Otel_Span_SetAttributeFn)(void* user_data,
    const TF_TString* key, const TF_TString* value);
typedef void (*TP_Otel_Span_SetStatusFn)(void* user_data,
    int status, const TF_TString* description);
typedef void (*TP_Otel_Span_EndFn)(void* user_data);

typedef struct TP_Otel_Span {
  size_t struct_size;
  void* ext;
  TP_Otel_Span_SetAttributeFn set_attribute_cb;
  TP_Otel_Span_SetStatusFn set_status_cb;
  TP_Otel_Span_EndFn end_cb;
#define TP_OTEL_SPAN_STRUCT_SIZE TF_OFFSET_OF_END(TP_Otel_Span, end_cb)
} TP_Otel_Span;

// LINT.ThenChange(:otel_span_version)



static inline TP_Otel_Span* TP_OtelSpanNew(void) {
  TP_Otel_Span* ptr = (TP_Otel_Span*)malloc(sizeof(struct TP_Otel_Span));
  if (!ptr) return nullptr;
  ptr->struct_size = sizeof(struct TP_Otel_Span);
  ptr->ext = nullptr;
  ptr->set_attribute_cb = nullptr;
  ptr->set_status_cb = nullptr;
  ptr->end_cb = nullptr;
  return ptr;
}

static inline void TP_OtelSpanDelete(TP_Otel_Span* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TP_OtelSpan_SetStructSize(TP_Otel_Span* builder, size_t struct_size) {
  builder->struct_size = struct_size;
}

static inline void TP_OtelSpan_SetExt(TP_Otel_Span* builder, void* ext) {
  builder->ext = ext;
}

static inline void TP_OtelSpan_SetSetAttributeCallback(TP_Otel_Span* builder, TP_Otel_Span_SetAttributeFn set_attribute_cb) {
  builder->set_attribute_cb = set_attribute_cb;
}

static inline void TP_OtelSpan_SetSetStatusCallback(TP_Otel_Span* builder, TP_Otel_Span_SetStatusFn set_status_cb) {
  builder->set_status_cb = set_status_cb;
}

static inline void TP_OtelSpan_SetEndCallback(TP_Otel_Span* builder, TP_Otel_Span_EndFn end_cb) {
  builder->end_cb = end_cb;
}



#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_OTEL_SPAN_H_