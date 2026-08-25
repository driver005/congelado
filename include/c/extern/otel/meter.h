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

#ifndef CONGELADO_C_OTEL_METER_H_
#define CONGELADO_C_OTEL_METER_H_

#include <stddef.h>
#include <stdlib.h>

#include "c/intern/tf_tstring.h"

#include "c/extern/otel/opaque_types.h"
#include "c/extern/otel/counter.h"
#include "c/extern/otel/histogram.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TP_Otel_Counter* (*TP_Otel_Meter_CreateCounterFn)(void* user_data,
    const TF_TString* name, const TF_TString* description,
    const TF_TString* unit);
typedef TP_Otel_Histogram* (*TP_Otel_Meter_CreateHistogramFn)(void* user_data,
    const TF_TString* name, const TF_TString* description,
    const TF_TString* unit);

typedef struct TP_Otel_Meter {
  size_t struct_size;
  void* ext;
  TP_Otel_Meter_CreateCounterFn create_counter_cb;
  TP_Otel_Meter_CreateHistogramFn create_histogram_cb;
#define TP_OTEL_METER_STRUCT_SIZE TF_OFFSET_OF_END(TP_Otel_Meter, create_histogram_cb)
} TP_Otel_Meter;

// LINT.ThenChange(:otel_meter_version)



static inline TP_Otel_Meter* TP_OtelMeterNew(void) {
  TP_Otel_Meter* ptr = (TP_Otel_Meter*)malloc(sizeof(struct TP_Otel_Meter));
  if (!ptr) return nullptr;
  ptr->struct_size = sizeof(struct TP_Otel_Meter);
  ptr->ext = nullptr;
  ptr->create_counter_cb = nullptr;
  ptr->create_histogram_cb = nullptr;
  return ptr;
}

static inline void TP_OtelMeterDelete(TP_Otel_Meter* ptr) {
  if (!ptr) return;
  free(ptr);
}

static inline void TP_OtelMeter_SetStructSize(TP_Otel_Meter* builder, size_t struct_size) {
  builder->struct_size = struct_size;
}

static inline void TP_OtelMeter_SetExt(TP_Otel_Meter* builder, void* ext) {
  builder->ext = ext;
}

static inline void TP_OtelMeter_SetCreateCounterCallback(TP_Otel_Meter* builder, TP_Otel_Meter_CreateCounterFn create_counter_cb) {
  builder->create_counter_cb = create_counter_cb;
}

static inline void TP_OtelMeter_SetCreateHistogramCallback(TP_Otel_Meter* builder, TP_Otel_Meter_CreateHistogramFn create_histogram_cb) {
  builder->create_histogram_cb = create_histogram_cb;
}



#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_OTEL_METER_H_