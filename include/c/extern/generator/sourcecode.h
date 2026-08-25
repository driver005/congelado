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

#ifndef CONGELADO_C_GENERATOR_SOURCECODE_H_
#define CONGELADO_C_GENERATOR_SOURCECODE_H_

#include "c/abi/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TF_Generator_SourceCode TF_Generator_SourceCode;

TF_CAPI_EXPORT extern TF_Generator_SourceCode* TF_Generator_SourceCode_Create(
    void);
TF_CAPI_EXPORT extern void TF_Generator_SourceCode_Destroy(
    TF_Generator_SourceCode* code);
TF_CAPI_EXPORT extern void TF_Generator_SourceCode_AddLine(
    TF_Generator_SourceCode* code, const char* line);
TF_CAPI_EXPORT extern void TF_Generator_SourceCode_AddBlankLine(
    TF_Generator_SourceCode* code);
TF_CAPI_EXPORT extern void TF_Generator_SourceCode_IncreaseIndent(
    TF_Generator_SourceCode* code);
TF_CAPI_EXPORT extern void TF_Generator_SourceCode_DecreaseIndent(
    TF_Generator_SourceCode* code);
TF_CAPI_EXPORT extern void TF_Generator_SourceCode_SetSpacesPerIndent(
    TF_Generator_SourceCode* code, int spaces);
TF_CAPI_EXPORT extern char* TF_Generator_SourceCode_Render(
    const TF_Generator_SourceCode* code);
TF_CAPI_EXPORT extern void TF_Generator_SourceCode_FreeRendered(char* rendered);

#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_GENERATOR_SOURCECODE_H_
