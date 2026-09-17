#ifndef TENSORFLOW_C_TF_COMPLEX_H_
#define TENSORFLOW_C_TF_COMPLEX_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Complex
    {
        void* plugin_data;
    } TF_Complex;

    typedef struct TF_ComplexOps
    {
        size_t struct_size;

        void (*get_real)(const TF_Complex* complex_value, double* out_real);
        void (*get_imag)(const TF_Complex* complex_value, double* out_imag);
        void (*set_real)(TF_Complex* complex_value, double real);
        void (*set_imag)(TF_Complex* complex_value, double imag);
        void (*destroy)(TF_Complex* complex_value);

    } TF_ComplexOps;

#define TF_COMPLEX_STRUCT_SIZE TF_OFFSET_OF_END(TF_ComplexOps, destroy)

    TF_CAPI_EXPORT void create_complex(TF_ComplexOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_complex(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_COMPLEX_H_
