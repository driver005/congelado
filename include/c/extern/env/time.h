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

#ifndef CONGELADO_C_ENV_TIME_H_
#define CONGELADO_C_ENV_TIME_H_

#include "c/abi/macros.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Returns the number of nanoseconds since the Unix epoch.
    TF_CAPI_EXPORT extern uint64_t TF_NowNanos(void);

    // Returns the number of microseconds since the Unix epoch.
    TF_CAPI_EXPORT extern uint64_t TF_NowMicros(void);

    // Returns the number of seconds since the Unix epoch.
    TF_CAPI_EXPORT extern uint64_t TF_NowSeconds(void);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_ENV_TIME_H_
