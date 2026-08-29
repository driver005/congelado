/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

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

#ifndef TENSORFLOW_C_TF_TSTRING_H_
#define TENSORFLOW_C_TF_TSTRING_H_

#include "c/abi/macros.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_TString types for small-string optimization.
    typedef enum TF_TString_Type
    {
        TF_TSTR_SMALL = 0,
        TF_TSTR_LARGE = 1,
        TF_TSTR_OFFSET = 2,
        TF_TSTR_VIEW = 3
    } TF_TString_Type;

    // Small-string-optimized string type.
    // Inline for small strings, heap-allocated for large ones.
    typedef union TF_TString
    {
        struct
        {
            uint8_t size;
            char data[23];
        } small;

        struct
        {
            size_t size;
            size_t capacity;
            char* data;
        } large;

        struct
        {
            uint16_t offset;
            uint16_t count;
            uint32_t dummy;
            const char* data;
        } offset;

        struct
        {
            size_t size;
            const char* data;
            char dummy[16];
        } view;
    } TF_TString;

    // Public C ABI spelling used by Congelado external interfaces.
    typedef TF_TString TF_String;

    TF_CAPI_EXPORT void string_init(TF_TString* t);

    TF_CAPI_EXPORT void string_copy(TF_TString* dst, const char* src, size_t size);

    TF_CAPI_EXPORT void string_assign_view(TF_TString* dst, const char* src, size_t size);

    TF_CAPI_EXPORT const char* string_get_data_pointer(const TF_TString* tstr);

    TF_CAPI_EXPORT TF_TString_Type string_get_type(const TF_TString* str);

    TF_CAPI_EXPORT size_t string_get_size(const TF_TString* tstr);

    TF_CAPI_EXPORT size_t string_get_capacity(const TF_TString* str);

    TF_CAPI_EXPORT void string_dealloc(TF_TString* tstr);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_TSTRING_H_
