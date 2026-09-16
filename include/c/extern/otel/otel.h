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
#ifndef CONGELADO_C_OTEL_CONTROLLER_H_
#define CONGELADO_C_OTEL_CONTROLLER_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct TF_Otel_Tracer TF_Otel_Tracer;
    typedef struct TF_Otel_Span TF_Otel_Span;
    typedef struct TF_Otel_Meter TF_Otel_Meter;
    typedef struct TF_Otel_Counter TF_Otel_Counter;
    typedef struct TF_Otel_Histogram TF_Otel_Histogram;

    typedef struct TF_Otel
    {
        void* plugin_data;
    } TF_Otel;

    typedef struct TF_OtelOps
    {
        size_t struct_size;
        void (*destroy)(TF_Otel* otel);
        void (*get_name)(TF_Otel* otel, TF_String* out);
        void (*tracer_destroy)(TF_Otel_Tracer* tracer_context);
        void (*meter_destroy)(TF_Otel_Meter* meter_context);
        void (*meter_destroy)(TF_Otel_Meter* meter_context);
        TF_Otel_Span* (*tracer_start_span)(
            TF_Otel_Tracer* tracer_context,
            const TF_String* name,
            int kind,
            TF_Status* status
        );
        void (*span_destroy)(TF_Otel_Span* span_context);
        void (*span_set_attribute)(
            TF_Otel_Span* span_context,
            const TF_String* key,
            const TF_String* value,
            TF_Status* status
        );
        void (*span_set_status)(
            TF_Otel_Span* span_context,
            int status_code,
            const TF_String* description,
            TF_Status* status
        );
        void (*span_end)(TF_Otel_Span* span_context, TF_Status* status);
        TF_Otel_Counter* (*meter_create_counter)(
            TF_Otel_Meter* meter_context,
            const TF_String* name,
            const TF_String* description,
            const TF_String* unit,
            TF_Status* status
        );
        void (*counter_destroy)(TF_Otel_Counter* counter_context);
        void (*counter_add)(TF_Otel_Counter* counter_context, double value, TF_Status* status);
        TF_Otel_Histogram* (*meter_create_histogram)(
            TF_Otel_Meter* meter_context,
            const TF_String* name,
            const TF_String* description,
            const TF_String* unit,
            TF_Status* status
        );
        void (*histogram_destroy)(TF_Otel_Histogram* histogram_context);
        void (*histogram_record)(TF_Otel_Histogram* histogram_context, double value, TF_Status* status);
    } TF_OtelOps;

#define TF_OTEL_STRUCT_SIZE TF_OFFSET_OF_END(TF_OtelOps, histogram_record)

    TF_CAPI_EXPORT void create_otel(TF_OtelOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_otel(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_OTEL_CONTROLLER_H_
