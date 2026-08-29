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
#ifndef CONGELADO_C_GENERATOR_CONTROLLER_H_
#define CONGELADO_C_GENERATOR_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/extern/generator/generator.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_tensor.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    TF_CAPI_EXPORT extern void TF_Generator_SetName(void* plugin_context, const TF_String* name);
    TF_CAPI_EXPORT extern void TF_Generator_GetName(void* plugin_context, TF_String* out);
    TF_CAPI_EXPORT extern TF_Tensor_Handle* TF_Generator_GetDefinitions(void* plugin_context, TF_Status* status);
    TF_CAPI_EXPORT extern bool TF_Generator_Build(void* plugin_context, TF_String* out, TF_Status* status);
    TF_CAPI_EXPORT extern void* TF_Generator_EnterBorderPatrol(void* plugin_context, const TF_String* name, TF_Status* status);

    TF_CAPI_EXPORT extern void TF_Generator_Function_Destroy(void* function_context);
    TF_CAPI_EXPORT extern void* TF_Generator_Function_AddParameter(void* function_context, const TF_String* name, const TF_String* type_text, TF_Status* status);
    TF_CAPI_EXPORT extern bool TF_Generator_Function_AddNode(
        void* function_context,
        const void* def_context,
        const TF_Tensor_Handle* operands,
        const TF_Tensor_Handle* attrs,
        TF_Tensor_Handle* out_results,
        TF_Status* status
    );
    TF_CAPI_EXPORT extern bool TF_Generator_Function_ExitBorderPatrol(
        void* function_context,
        const TF_Tensor_Handle* outputs,
        TF_Status* status
    );

    TF_CAPI_EXPORT extern void TF_Generator_Definition_Destroy(void* def_context);
    TF_CAPI_EXPORT extern void TF_Generator_Definition_GetName(void* def_context, TF_String* out);
    TF_CAPI_EXPORT extern void TF_Generator_Definition_GetSummary(void* def_context, TF_String* out);
    TF_CAPI_EXPORT extern void TF_Generator_Definition_GetDescription(void* def_context, TF_String* out);
    TF_CAPI_EXPORT extern TF_Tensor_Handle* TF_Generator_Definition_GetInputs(void* def_context, TF_Status* status);
    TF_CAPI_EXPORT extern TF_Tensor_Handle* TF_Generator_Definition_GetOutputs(void* def_context, TF_Status* status);
    TF_CAPI_EXPORT extern TF_Tensor_Handle* TF_Generator_Definition_GetAttrs(void* def_context, TF_Status* status);

    TF_CAPI_EXPORT extern void TF_Generator_TypeInfo_Destroy(void* type_context);
    TF_CAPI_EXPORT extern int TF_Generator_TypeInfo_GetDataType(void* type_context);
    TF_CAPI_EXPORT extern void TF_Generator_TypeInfo_GetTypeAttrName(void* type_context, TF_String* out);
    TF_CAPI_EXPORT extern bool TF_Generator_TypeInfo_IsReadOnly(void* type_context);
    TF_CAPI_EXPORT extern bool TF_Generator_TypeInfo_IsList(void* type_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_GENERATOR_CONTROLLER_H_
