/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

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

#ifndef CONGELADO_C_GENERATOR_CONTROLLER_H_
#define CONGELADO_C_GENERATOR_CONTROLLER_H_

#include <stddef.h>

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TF_Generator_Controller TF_Generator_Controller;
typedef struct TF_Generator_Definition TF_Generator_Definition;
typedef struct TF_Generator_SourceCode TF_Generator_SourceCode;

TF_CAPI_EXPORT extern TF_Generator_Controller* TF_Generator_Create(
    const char* output_dir, const char* source_dir, TF_Status* status);
TF_CAPI_EXPORT extern void TF_Generator_Destroy(
    TF_Generator_Controller* controller);
// The generator's identity — set by the caller (e.g. "stablehlo") so a registry can
// hold more than one generator. `name` is copied; `out` is assigned a view into the
// stored name (caller owns init/destroy of `out`).
TF_CAPI_EXPORT extern void TF_Generator_SetName(
    TF_Generator_Controller* controller, const TF_String* name);
TF_CAPI_EXPORT extern void TF_Generator_GetName(
    const TF_Generator_Controller* controller, TF_String* out);
TF_CAPI_EXPORT extern size_t TF_Generator_GetDefinitionCount(
    const TF_Generator_Controller* controller);
TF_CAPI_EXPORT extern const TF_Generator_Definition* TF_Generator_GetDefinition(
    const TF_Generator_Controller* controller, size_t index);
TF_CAPI_EXPORT extern void TF_Generator_WriteFile(
    TF_Generator_Controller* controller, const char* file_path,
    const TF_Generator_SourceCode* code, TF_Status* status);
TF_CAPI_EXPORT extern void TF_Generator_WriteModule(
    TF_Generator_Controller* controller, const char* file_path, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_GENERATOR_CONTROLLER_H_
