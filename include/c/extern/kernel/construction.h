#ifndef TENSORFLOW_C_EXTERN_KERNEL_CONSTRUCTION_H_
#define TENSORFLOW_C_EXTERN_KERNEL_CONSTRUCTION_H_

#include "include/c/macros.h"
#include "include/c/intern/datatype.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/buffer.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_OpKernelConstruction { void* plugin_data; } TF_OpKernelConstruction;

    // TF_OpKernelConstructionOps
    typedef struct TF_OpKernelConstructionOps {
        size_t struct_size;
        void (*failure)(TF_OpKernelConstruction* ctx, TF_Status* out_status);
        void (*get_node_def)(TF_OpKernelConstruction* ctx, TF_Buffer* buffer, TF_Status* out_status);
        void (*get_attr_size)(TF_OpKernelConstruction* ctx, const char* attr_name, int32_t* out_list_size, int32_t* out_total_size, TF_Status* out_status);
        void (*get_attr_type)(TF_OpKernelConstruction* ctx, const char* attr_name, TFDataTypeEnum* out_val, TF_Status* out_status);
        void (*get_attr_int32)(TF_OpKernelConstruction* ctx, const char* attr_name, int32_t* out_val, TF_Status* out_status);
        void (*get_attr_int64)(TF_OpKernelConstruction* ctx, const char* attr_name, int64_t* out_val, TF_Status* out_status);
        void (*get_attr_float)(TF_OpKernelConstruction* ctx, const char* attr_name, float* out_val, TF_Status* out_status);
        void (*get_attr_bool)(TF_OpKernelConstruction* ctx, const char* attr_name, bool* out_val, TF_Status* out_status);
        void (*get_attr_string)(TF_OpKernelConstruction* ctx, const char* attr_name, char* out_val, size_t max_length, TF_Status* out_status);
        void (*get_attr_tensor)(TF_OpKernelConstruction* ctx, const char* attr_name, TF_Tensor** out_val, TF_Status* out_status);
        void (*get_attr_type_list)(TF_OpKernelConstruction* ctx, const char* attr_name, TFDataTypeEnum* out_vals, int max_vals, TF_Status* out_status);
        void (*get_attr_int32_list)(TF_OpKernelConstruction* ctx, const char* attr_name, int32_t* out_vals, int max_vals, TF_Status* out_status);
        void (*get_attr_int64_list)(TF_OpKernelConstruction* ctx, const char* attr_name, int64_t* out_vals, int max_vals, TF_Status* out_status);
        void (*get_attr_float_list)(TF_OpKernelConstruction* ctx, const char* attr_name, float* out_vals, int max_vals, TF_Status* out_status);
        void (*get_attr_bool_list)(TF_OpKernelConstruction* ctx, const char* attr_name, bool* out_vals, int max_vals, TF_Status* out_status);
        void (*get_attr_string_list)(TF_OpKernelConstruction* ctx, const char* attr_name, char** out_values, size_t* out_lengths, int max_values, void* storage, size_t storage_size, TF_Status* out_status);
        void (*get_attr_tensor_list)(TF_OpKernelConstruction* ctx, const char* attr_name, TF_Tensor** out_vals, int max_values, TF_Status* out_status);
        void (*get_attr_function)(TF_OpKernelConstruction* ctx, const char* attr_name, TF_Buffer* buffer, TF_Status* out_status);
        void (*has_attr)(TF_OpKernelConstruction* ctx, const char* attr_name, bool* out_has_attr, TF_Status* out_status);
        void (*get_name)(TF_OpKernelConstruction* ctx, TF_String* out_name);
        void (*get_attr_tensor_shape)(TF_OpKernelConstruction* ctx, const char* attr_name, int64_t* out_dims, size_t num_dims, TF_Status* out_status);
    } TF_OpKernelConstructionOps;
    #define TF_OP_KERNEL_CONSTRUCTION_STRUCT_SIZE TF_OFFSET_OF_END(TF_OpKernelConstructionOps, get_attr_tensor_shape)
    TF_CAPI_EXPORT void create_op_kernel_construction(TF_OpKernelConstructionOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_op_kernel_construction(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_KERNEL_CONSTRUCTION_H_
