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
#ifndef CONGELADO_C_GENERATOR_ATTRIBUTE_H_
#define CONGELADO_C_GENERATOR_ATTRIBUTE_H_

#include "c/abi/macros.h"
#include "c/extern/generator/generator.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    TF_CAPI_EXPORT extern void TF_Generator_Attribute_GetName(void* attr_context, TF_String* out);
    TF_CAPI_EXPORT extern void TF_Generator_Attribute_GetDescription(void* attr_context, TF_String* out);
    TF_CAPI_EXPORT extern void TF_Generator_Attribute_GetFullType(void* attr_context, TF_String* out);
    TF_CAPI_EXPORT extern void TF_Generator_Attribute_GetBaseType(void* attr_context, TF_String* out);
    TF_CAPI_EXPORT extern bool TF_Generator_Attribute_IsList(void* attr_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_GENERATOR_ATTRIBUTE_H_
