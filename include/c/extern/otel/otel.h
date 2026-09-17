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
#ifndef CONGELADO_C_OTEL_H_
#define CONGELADO_C_OTEL_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include "c/extern/otel/tracer.h"
#include "c/extern/otel/span.h"
#include "c/extern/otel/meter.h"
#include "c/extern/otel/counter.h"
#include "c/extern/otel/histogram.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Otel
    {
        void* plugin_data;
        const TFOtelTracerOps* tracer_ops;
        const TFOtelMeterOps* meter_ops;
        const TFOtelCounterOps* counter_ops;
        const TFOtelHistogramOps* histogram_ops;
    } TF_Otel;

    typedef struct TF_OtelOps
    {
        size_t struct_size;
        void (*destroy)(TF_Otel* otel);
        void (*get_name)(TF_Otel* otel, TF_String* out_name);
    } TF_OtelOps;

#define TF_OTEL_STRUCT_SIZE TF_OFFSET_OF_END(TF_OtelOps, get_name)

    TF_CAPI_EXPORT void create_otel(TF_OtelOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_otel(void* plugin_context);

    static inline void init_otel(TF_OtelOps** ops, TF_Otel* otel, TF_Status* out_status)
    {
        create_otel(ops, &otel->plugin_data, out_status);

        TFOtelTracerOps* tracer_ops = NULL;
        create_otel_tracer(&tracer_ops, &otel->plugin_data, out_status);
        otel->tracer_ops = tracer_ops;

        TFOtelMeterOps* meter_ops = NULL;
        create_otel_meter(&meter_ops, &otel->plugin_data, out_status);
        otel->meter_ops = meter_ops;

        TFOtelCounterOps* counter_ops = NULL;
        create_otel_counter(&counter_ops, &otel->plugin_data, out_status);
        otel->counter_ops = counter_ops;

        TFOtelHistogramOps* histogram_ops = NULL;
        create_otel_histogram(&histogram_ops, &otel->plugin_data, out_status);
        otel->histogram_ops = histogram_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_OTEL_H_
