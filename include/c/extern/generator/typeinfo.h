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

#ifndef CONGELADO_C_GENERATOR_TYPEINFO_H_
#define CONGELADO_C_GENERATOR_TYPEINFO_H_

#include <stdbool.h>

#include "c/abi/macros.h"
#include "c/intern/tf_datatype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TF_Generator_TypeInfo TF_Generator_TypeInfo;

TF_CAPI_EXPORT extern int TF_Generator_TypeInfo_GetDataType(
    const TF_Generator_TypeInfo* type);
TF_CAPI_EXPORT extern const char* TF_Generator_TypeInfo_GetTypeAttrName(
    const TF_Generator_TypeInfo* type);
TF_CAPI_EXPORT extern bool TF_Generator_TypeInfo_IsReadOnly(
    const TF_Generator_TypeInfo* type);
TF_CAPI_EXPORT extern bool TF_Generator_TypeInfo_IsList(
    const TF_Generator_TypeInfo* type);

#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_GENERATOR_TYPEINFO_H_
