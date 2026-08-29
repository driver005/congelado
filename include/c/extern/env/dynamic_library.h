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
#ifndef CONGELADO_C_ENV_DYNAMIC_LIBRARY_H_
#define CONGELADO_C_ENV_DYNAMIC_LIBRARY_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#ifdef __cplusplus
extern "C"
{
#endif

    TF_CAPI_EXPORT void* load_shared_library(const char* library_filename, TF_Status* status);
    TF_CAPI_EXPORT void*
    get_symbol_from_library(void* handle, const char* symbol_name, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_ENV_DYNAMIC_LIBRARY_H_
