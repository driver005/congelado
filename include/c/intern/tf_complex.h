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

#ifndef TENSORFLOW_C_TF_COMPLEX_H_
#define TENSORFLOW_C_TF_COMPLEX_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Complex — plugin vtable for a two-component complex number
    // (std::complex<T> equivalent). Components are always carried as double;
    // is_double, given at creation, records whether the logical precision is
    // float or double for callers that care.
    //
    // TF_Complex_Handle is an opaque pointer to a plugin-owned complex object.
    typedef struct TF_Complex_Handle TF_Complex_Handle;

    // Plugin-facing vtable registered via init_complex.
    typedef struct TF_Complex
    {
        size_t struct_size;

        // Allocate a new complex number real + imag*i. is_double is
        // non-zero if the logical precision is double rather than float.
        // Must be freed with destroy.
        TF_Complex_Handle* (*new_complex)(
            void* plugin_context,
            int is_double,
            double real,
            double imag
        );

        // The real component.
        double (*get_real)(void* plugin_context, const TF_Complex_Handle* complex_value);

        // The imaginary component.
        double (*get_imag)(void* plugin_context, const TF_Complex_Handle* complex_value);

        // Overwrite the real component.
        void (*set_real)(void* plugin_context, TF_Complex_Handle* complex_value, double real);

        // Overwrite the imaginary component.
        void (*set_imag)(void* plugin_context, TF_Complex_Handle* complex_value, double imag);

        // Free a handle returned by new_complex.
        void (*destroy)(void* plugin_context, TF_Complex_Handle* complex_value);

    } TF_Complex;

#define TF_COMPLEX_STRUCT_SIZE TF_OFFSET_OF_END(TF_Complex, destroy)

    TF_CAPI_EXPORT void init_complex(TF_Complex** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_COMPLEX_H_
