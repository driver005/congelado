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
#ifndef CONGELADO_C_FILESYSTEM_OPTION_TYPES_H_
#define CONGELADO_C_FILESYSTEM_OPTION_TYPES_H_

#include "c/abi/macros.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum TF_Filesystem_Option_Type
    {
        TF_FILESYSTEM_OPTION_TYPE_INT = 0,
        TF_FILESYSTEM_OPTION_TYPE_REAL = 1,
        TF_FILESYSTEM_OPTION_TYPE_BUFFER = 2,
    } TF_Filesystem_Option_Type;

    typedef struct TF_Filesystem_Option_Value
    {
        TF_Filesystem_Option_Type type;

        union
        {
            int64_t int_value;
            double real_value;

            struct
            {
                const char* data;
                size_t size;
            } buffer_value;
        };
    } TF_Filesystem_Option_Value;

    typedef struct TF_Filesystem_Option
    {
        TF_Filesystem_Option_Type type_tag;
        const char* name;
        const char* description;
        TF_Filesystem_Option_Value value;
    } TF_Filesystem_Option;

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_FILESYSTEM_OPTION_TYPES_H_
