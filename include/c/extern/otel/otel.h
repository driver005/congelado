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

#include "c/abi/macros.h"
#include "c/extern/otel/enums.h"
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
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        TF_Otel_Tracer* (*create_tracer)(void* plugin_context, TF_Status* status);
        void (*tracer__destroy)(TF_Otel_Tracer* tracer_context);
        TF_Otel_Meter* (*create_meter)(void* plugin_context, TF_Status* status);
        void (*meter__destroy)(TF_Otel_Meter* meter_context);
        TF_Otel_Span* (*tracer__start_span)(
            TF_Otel_Tracer* tracer_context,
            const TF_TString* name,
            int kind,
            TF_Status* status
        );
        void (*span__destroy)(TF_Otel_Span* span_context);
        void (*span__set_attribute)(
            TF_Otel_Span* span_context,
            const TF_TString* key,
            const TF_TString* value,
            TF_Status* status
        );
        void (*span__set_status)(
            TF_Otel_Span* span_context,
            int status_code,
            const TF_TString* description,
            TF_Status* status
        );
        void (*span__end)(TF_Otel_Span* span_context, TF_Status* status);
        TF_Otel_Counter* (*meter__create_counter)(
            TF_Otel_Meter* meter_context,
            const TF_TString* name,
            const TF_TString* description,
            const TF_TString* unit,
            TF_Status* status
        );
        void (*counter__destroy)(TF_Otel_Counter* counter_context);
        void (*counter__add)(TF_Otel_Counter* counter_context, double value, TF_Status* status);
        TF_Otel_Histogram* (*meter__create_histogram)(
            TF_Otel_Meter* meter_context,
            const TF_TString* name,
            const TF_TString* description,
            const TF_TString* unit,
            TF_Status* status
        );
        void (*histogram__destroy)(TF_Otel_Histogram* histogram_context);
        void (*histogram__record)(TF_Otel_Histogram* histogram_context, double value, TF_Status* status);
    } TF_Otel;

#define TF_OTEL_STRUCT_SIZE TF_OFFSET_OF_END(TF_Otel, histogram__record)

    TF_CAPI_EXPORT void init_otel(TF_Otel** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_OTEL_CONTROLLER_H_
