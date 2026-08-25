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

#ifndef CONGELADO_C_OTEL_HISTOGRAM_H_
#define CONGELADO_C_OTEL_HISTOGRAM_H_

#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*TP_Otel_Histogram_RecordFn)(void* user_data, double value);

typedef struct TP_Otel_Histogram {
  size_t struct_size;
  void* ext;
  TP_Otel_Histogram_RecordFn record_cb;
#define TP_OTEL_HISTOGRAM_STRUCT_SIZE TF_OFFSET_OF_END(TP_Otel_Histogram, record_cb)
} TP_Otel_Histogram;

// LINT.ThenChange(:otel_histogram_version)



static inline TP_Otel_Histogram* TP_OtelHistogramNew(void) {
  TP_Otel_Histogram* ptr = (TP_Otel_Histogram*)malloc(sizeof(struct TP_Otel_Histogram));
  if (!ptr) return nullptr;
  ptr->struct_size = sizeof(struct TP_Otel_Histogram);
  ptr->ext = nullptr;
  ptr->record_cb = nullptr;
  return ptr;
}

static inline void TP_OtelHistogramDelete(TP_Otel_Histogram* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TP_OtelHistogram_SetStructSize(TP_Otel_Histogram* builder, size_t struct_size) {
  builder->struct_size = struct_size;
}

static inline void TP_OtelHistogram_SetExt(TP_Otel_Histogram* builder, void* ext) {
  builder->ext = ext;
}

static inline void TP_OtelHistogram_SetRecordCallback(TP_Otel_Histogram* builder, TP_Otel_Histogram_RecordFn record_cb) {
  builder->record_cb = record_cb;
}



#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_OTEL_HISTOGRAM_H_