#ifndef TENSORFLOW_C_EXTERN_GRADIENTS_GRADIENT_H_
#define TENSORFLOW_C_EXTERN_GRADIENTS_GRADIENT_H_

#include "include/c/extern/gradients/context.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // The gradient of one op (TensorFlow's GradientFunction). The plugin registers the ops table of
    // each gradient in the global registry (include/c/intern/registration.h) with type "gradient"
    // and the op it differentiates as provider; the host creates it and calls compute. An op with
    // no gradient (TensorFlow's RegisterNotDifferentiable) gets a gradient whose compute sets none.
    typedef struct TFGradient
    {
        void* plugin_data;
        const TFGradientContextOps* context_ops;
    } TFGradient;

    // TFGradientOps
    typedef struct TFGradientOps
    {
        size_t struct_size;
        void (*create)(TFGradient* out_handle);
        void (*destroy)(TFGradient* handle);
        void (*get_name)(TFGradient* gradient, TF_String* out_name);
        void (*compute)(TFGradient* gradient, TFGradientContext* ctx, TF_Status* out_status);
    } TFGradientOps;

#define TF_GRADIENT_STRUCT_SIZE TF_OFFSET_OF_END(TFGradientOps, compute)

    TF_CAPI_EXPORT void
    create_gradient(TFGradientOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_gradient(void* plugin_context);

    static inline void
    init_gradient(TFGradientOps** ops, TFGradient* gradient, TF_Status* out_status)
    {
        create_gradient(ops, &gradient->plugin_data, out_status);

        TFGradientContextOps* context_ops = NULL;
        create_gradient_context(&context_ops, &gradient->plugin_data, out_status);
        gradient->context_ops = context_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_GRADIENTS_GRADIENT_H_
