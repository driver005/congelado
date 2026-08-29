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
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct TF_Otel_Meter_Controller TF_Otel_Meter_Controller;
    typedef struct TF_Otel_Span_Controller TF_Otel_Span_Controller;
    typedef struct TF_Otel_Counter_Controller TF_Otel_Counter_Controller;
    typedef struct TF_Otel_Histogram_Controller TF_Otel_Histogram_Controller;

    
    

    
    

    
    

    
    

    
    
    

    
    
    

    
    
    

    typedef struct TF_Otel {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void* (*get_tracer)(void* plugin_context, TF_Status* status);
        void (*tracer__destroy)(void* tracer);
        void* (*get_meter)(void* plugin_context, TF_Status* status);
        void (*meter__destroy)(void* meter);
        void* (*tracer__start_span)(void* tracer, const TF_TString* name, int kind, TF_Status* status);
        void (*span__destroy)(void* span);
        void (*span__set_attribute)(void* span, const TF_TString* key, const TF_TString* value, TF_Status* status);
        void (*span__set_status)(void* span, int status_code, const TF_TString* description, TF_Status* status);
        void (*span__end)(void* span, TF_Status* status);
        void* (*meter__create_counter)(void* meter, const TF_TString* name, const TF_TString* description, const TF_TString* unit, TF_Status* status);
        void (*counter__destroy)(void* counter);
        void (*counter__add)(void* counter, double value, TF_Status* status);
        void* (*meter__create_histogram)(void* meter, const TF_TString* name, const TF_TString* description, const TF_TString* unit, TF_Status* status);
        void (*histogram__destroy)(void* histogram);
        void (*histogram__record)(void* histogram, double value, TF_Status* status);
    } TF_Otel;

    TF_CAPI_EXPORT extern void TF_InitOtel(TF_Otel** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_OTEL_CONTROLLER_H_
