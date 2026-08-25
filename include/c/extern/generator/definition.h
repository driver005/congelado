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

#ifndef CONGELADO_C_GENERATOR_DEFINITION_H_
#define CONGELADO_C_GENERATOR_DEFINITION_H_

#include <stddef.h>

#include "c/abi/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TF_Generator_Definition TF_Generator_Definition;
typedef struct TF_Generator_Parameter TF_Generator_Parameter;
typedef struct TF_Generator_Attribute TF_Generator_Attribute;

TF_CAPI_EXPORT extern const char* TF_Generator_Definition_GetName(
    const TF_Generator_Definition* def);
TF_CAPI_EXPORT extern const char* TF_Generator_Definition_GetSummary(
    const TF_Generator_Definition* def);
TF_CAPI_EXPORT extern const char* TF_Generator_Definition_GetDescription(
    const TF_Generator_Definition* def);
TF_CAPI_EXPORT extern size_t TF_Generator_Definition_GetInputCount(
    const TF_Generator_Definition* def);
TF_CAPI_EXPORT extern const TF_Generator_Parameter*
TF_Generator_Definition_GetInput(const TF_Generator_Definition* def, size_t index);
TF_CAPI_EXPORT extern size_t TF_Generator_Definition_GetOutputCount(
    const TF_Generator_Definition* def);
TF_CAPI_EXPORT extern const TF_Generator_Parameter*
TF_Generator_Definition_GetOutput(const TF_Generator_Definition* def, size_t index);
TF_CAPI_EXPORT extern size_t TF_Generator_Definition_GetAttrCount(
    const TF_Generator_Definition* def);
TF_CAPI_EXPORT extern const TF_Generator_Attribute*
TF_Generator_Definition_GetAttr(const TF_Generator_Definition* def, size_t index);

#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_GENERATOR_DEFINITION_H_
