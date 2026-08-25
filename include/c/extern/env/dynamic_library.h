/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

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
extern "C" {
#endif

// \brief Load a dynamic library.
//
// Pass "library_filename" to a platform-specific mechanism for dynamically
// loading a library. The rules for determining the exact location of the
// library are platform-specific and are not documented here.
//
// On success, place OK in status and return the newly created library handle.
// Otherwise returns nullptr and set error status.
TF_CAPI_EXPORT extern void* TF_LoadSharedLibrary(const char* library_filename,
                                                 TF_Status* status);

// \brief Get a pointer to a symbol from a dynamic library.
//
// "handle" should be a pointer returned from a previous call to
// TF_LoadLibraryFromEnv. On success, place OK in status and return a pointer to
// the located symbol. Otherwise returns nullptr and set error status.
TF_CAPI_EXPORT extern void* TF_GetSymbolFromLibrary(void* handle,
                                                    const char* symbol_name,
                                                    TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_ENV_DYNAMIC_LIBRARY_H_
