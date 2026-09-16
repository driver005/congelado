#ifndef TENSORFLOW_C_TF_COMPLEX_H_
#define TENSORFLOW_C_TF_COMPLEX_H_

#include "c/macros.h"
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
        double (*get_real)(const TF_Complex_Handle* complex_value);

        // The imaginary component.
        double (*get_imag)(const TF_Complex_Handle* complex_value);

        // Overwrite the real component.
        void (*set_real)(TF_Complex_Handle* complex_value, double real);

        // Overwrite the imaginary component.
        void (*set_imag)(TF_Complex_Handle* complex_value, double imag);

        // Free a handle returned by new_complex.
        void (*destroy)(void* plugin_context, TF_Complex_Handle* complex_value);

    } TF_Complex;

#define TF_COMPLEX_STRUCT_SIZE TF_OFFSET_OF_END(TF_Complex, destroy)

    TF_CAPI_EXPORT void init_complex(TF_Complex** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_COMPLEX_H_
