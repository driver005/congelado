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

#ifndef CONGELADO_C_OTEL_REGISTRATION_H_
#define CONGELADO_C_OTEL_REGISTRATION_H_

#include "c/extern/otel/counter.h"
#include "c/extern/otel/histogram.h"
#include "c/extern/otel/meter.h"
#include "c/extern/otel/span.h"
#include "c/extern/otel/tracer.h"
#include "c/intern/tf_bool.h"
#include "c/intern/tf_status.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
typedef struct TP_Otel TP_Otel;

#ifdef __cplusplus
extern "C"
{
#endif

    typedef TP_Otel_Tracer* (*TP_Otel_GetTracerProviderFn)(void* user_data);
    typedef TP_Otel_Meter* (*TP_Otel_GetMeterProviderFn)(void* user_data);

    typedef void (*TF_OtelRegistrationParams_DestroyOtel)(TP_Otel* otel);
    typedef void (*TF_OtelRegistrationParams_DestroyTracer)(void* tracer);
    typedef void (*TF_OtelRegistrationParams_DestroyMeter)(void* meter);
    typedef void (*TF_OtelRegistrationParams_DestroySpan)(void* span);
    typedef void (*TF_OtelRegistrationParams_DestroyCounter)(void* counter);
    typedef void (*TF_OtelRegistrationParams_DestroyHistogram)(void* histogram);

    typedef struct TP_Otel
    {
        size_t struct_size;
        void* ext;
        TP_Otel_GetTracerProviderFn get_tracer_provider_cb;
        TP_Otel_GetMeterProviderFn get_meter_provider_cb;
#define TP_OTEL_STRUCT_SIZE TF_OFFSET_OF_END(TP_Otel, get_meter_provider_cb)
    } TP_Otel;

    typedef struct TF_OtelRegistrationParams
    {
        size_t struct_size;
        void* ext;
        int32_t major_version;
        int32_t minor_version;
        int32_t patch_version;
        void* otel;
        TF_OtelRegistrationParams_DestroyOtel destroy_otel;
        TF_OtelRegistrationParams_DestroyTracer destroy_tracer;
        TF_OtelRegistrationParams_DestroyMeter destroy_meter;
        TF_OtelRegistrationParams_DestroySpan destroy_span;
        TF_OtelRegistrationParams_DestroyCounter destroy_counter;
        TF_OtelRegistrationParams_DestroyHistogram destroy_histogram;
#define TF_OTEL_REGISTRATION_PARAMS_STRUCT_SIZE                                                    \
    TF_OFFSET_OF_END(TF_OtelRegistrationParams, destroy_histogram)
    } TF_OtelRegistrationParams;

    static inline TP_Otel* TP_OtelNew(void)
    {
        TP_Otel* ptr = (TP_Otel*)malloc(sizeof(struct TP_Otel));
        if (!ptr) {
            return nullptr;
        }
        ptr->struct_size = sizeof(struct TP_Otel);
        ptr->ext = nullptr;
        ptr->get_tracer_provider_cb = nullptr;
        ptr->get_meter_provider_cb = nullptr;
        return ptr;
    }

    static inline void TP_OtelDelete(TP_Otel* ptr)
    {
        if (!ptr) {
            return;
        }
        free(ptr);
    }

    static inline void TP_Otel_SetGetTracerProviderCallback(
        TP_Otel* builder, TP_Otel_GetTracerProviderFn get_tracer_provider_cb
    )
    {
        builder->get_tracer_provider_cb = get_tracer_provider_cb;
    }

    static inline void TP_Otel_SetGetMeterProviderCallback(
        TP_Otel* builder, TP_Otel_GetMeterProviderFn get_meter_provider_cb
    )
    {
        builder->get_meter_provider_cb = get_meter_provider_cb;
    }

#define OT_MAJOR 0
#define OT_MINOR 0
#define OT_PATCH 1

    void TF_InitOtel(void* params, void* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_OTEL_REGISTRATION_H_
