#ifndef TENSORFLOW_C_EXTERN_GRADIENTS_CONTEXT_H_
#define TENSORFLOW_C_EXTERN_GRADIENTS_CONTEXT_H_

#include "include/c/intern/datatype.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // One gradient computation as the host sets it up: the forward op, the upstream gradients, the
    // slots for the results and a builder for the ops that compute them. It replaces TensorFlow's
    // ForwardOperation, AbstractContext and ops:: wrappers. The tensors are symbolic tensors of the
    // graph being built; one handed out through a TF_Tensor** slot is released with delete_tensor.
    typedef struct TFGradientContext
    {
        void* plugin_data;
    } TFGradientContext;

    // TFGradientContextOps
    typedef struct TFGradientContextOps
    {
        size_t struct_size;
        void (*create)(TFGradientContext* out_handle);
        void (*destroy)(TFGradientContext* handle);
        void (*get_name)(TFGradientContext* ctx, TF_String* out_name);
        void (*num_inputs)(TFGradientContext* ctx, int* out_num);
        void (*num_outputs)(TFGradientContext* ctx, int* out_num);
        void (*get_input)(
            TFGradientContext* ctx,
            int i,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
        void (*get_output)(
            TFGradientContext* ctx,
            int i,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
        void (*has_attr)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            bool* out_has_attr,
            TF_Status* out_status
        );
        void (*get_attr_size)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            int32_t* out_list_size,
            int32_t* out_total_size,
            TF_Status* out_status
        );
        void (*get_attr_type)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            TFDataTypeEnum* out_val,
            TF_Status* out_status
        );
        void (*get_attr_int32)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            int32_t* out_val,
            TF_Status* out_status
        );
        void (*get_attr_int64)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            int64_t* out_val,
            TF_Status* out_status
        );
        void (*get_attr_float)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            float* out_val,
            TF_Status* out_status
        );
        void (*get_attr_bool)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            bool* out_val,
            TF_Status* out_status
        );
        void (*get_attr_string)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            TF_String* out_val,
            TF_Status* out_status
        );
        void (*get_attr_int64_list)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            int64_t* out_vals,
            int max_vals,
            TF_Status* out_status
        );
        void (*num_grad_outputs)(TFGradientContext* ctx, int* out_num);
        void (*num_grad_inputs)(TFGradientContext* ctx, int* out_num);
        void (*get_grad_output)(
            TFGradientContext* ctx,
            int i,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
        void (*set_grad_input)(
            TFGradientContext* ctx,
            int i,
            const TF_Tensor* tensor,
            TF_Status* out_status
        );
        void (*begin_op)(
            TFGradientContext* ctx,
            const TF_String* op_name,
            const TF_String* name,
            TF_Status* out_status
        );
        void (*add_input)(TFGradientContext* ctx, const TF_Tensor* tensor, TF_Status* out_status);
        void (*set_attr_type)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            TFDataTypeEnum val,
            TF_Status* out_status
        );
        void (*set_attr_int32)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            int32_t val,
            TF_Status* out_status
        );
        void (*set_attr_int64)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            int64_t val,
            TF_Status* out_status
        );
        void (*set_attr_float)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            float val,
            TF_Status* out_status
        );
        void (*set_attr_bool)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            bool val,
            TF_Status* out_status
        );
        void (*set_attr_string)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            const TF_String* val,
            TF_Status* out_status
        );
        void (*set_attr_int64_list)(
            TFGradientContext* ctx,
            const TF_String* attr_name,
            const int64_t* vals,
            int num_vals,
            TF_Status* out_status
        );
        void (*finish_op)(TFGradientContext* ctx, int* out_num_outputs, TF_Status* out_status);
        void (*get_op_output)(
            TFGradientContext* ctx,
            int i,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
        void (*constant_scalar)(
            TFGradientContext* ctx,
            double val,
            TFDataTypeEnum dtype,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
    } TFGradientContextOps;

#define TF_GRADIENT_CONTEXT_STRUCT_SIZE TF_OFFSET_OF_END(TFGradientContextOps, constant_scalar)

    TF_CAPI_EXPORT void create_gradient_context(
        TFGradientContextOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_gradient_context(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_GRADIENTS_CONTEXT_H_
