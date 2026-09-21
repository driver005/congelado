import os

files = {
    'include/c/extern/kernel/builder.h': '''#ifndef TENSORFLOW_C_EXTERN_KERNEL_BUILDER_H_
#define TENSORFLOW_C_EXTERN_KERNEL_BUILDER_H_

#include "include/c/macros.h"
#include "include/c/intern/datatype.h"
#include "include/c/intern/status.h"
#include "include/c/extern/kernel/construction.h"
#include "include/c/extern/kernel/context.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_KernelBuilder { void* plugin_data; } TF_KernelBuilder;

    // TF_KernelBuilderOps
    typedef struct TF_KernelBuilderOps {
        size_t struct_size;
        void (*type_constraint)(TF_KernelBuilder* kernel_builder, const char* attr_name, TFDataTypeEnum type, TF_Status* out_status);
        void (*host_memory)(TF_KernelBuilder* kernel_builder, const char* arg_name);
        void (*priority)(TF_KernelBuilder* kernel_builder, int32_t priority_number);
        void (*label)(TF_KernelBuilder* kernel_builder, const char* label);
        void (*register_kernel_builder)(TF_KernelBuilder* builder, const char* kernel_name, TF_Status* out_status);
        void (*register_kernel_builder_with_kernel_def)(TF_KernelBuilder* builder, const char* serialized_kernel_def, const char* name, TF_Status* out_status);
    } TF_KernelBuilderOps;
    #define TF_KERNEL_BUILDER_STRUCT_SIZE TF_OFFSET_OF_END(TF_KernelBuilderOps, register_kernel_builder_with_kernel_def)
    
    TF_CAPI_EXPORT void create_kernel_builder(TF_KernelBuilderOps** ops, void** plugin_context, const char* op_name, const char* device_name, void (*create_func)(TF_OpKernelConstruction*, void** out_plugin_data), void (*compute_func)(void*, TF_OpKernelContext*), void (*delete_func)(void*), TF_Status* out_status);
    TF_CAPI_EXPORT void create_async_kernel_builder(TF_KernelBuilderOps** ops, void** plugin_context, const char* op_name, const char* device_name, void (*create_func)(TF_OpKernelConstruction*, void** out_plugin_data), void (*compute_async_func)(void*, TF_OpKernelContext*, void*), void (*delete_func)(void*), TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_kernel_builder(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_KERNEL_BUILDER_H_
''',

    'include/c/extern/kernel/construction.h': '''#ifndef TENSORFLOW_C_EXTERN_KERNEL_CONSTRUCTION_H_
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
        void (*get_attr_bool)(TF_OpKernelConstruction* ctx, const char* attr_name, TF_Bool* out_val, TF_Status* out_status);
        void (*get_attr_string)(TF_OpKernelConstruction* ctx, const char* attr_name, char* out_val, size_t max_length, TF_Status* out_status);
        void (*get_attr_tensor)(TF_OpKernelConstruction* ctx, const char* attr_name, TF_Tensor** out_val, TF_Status* out_status);
        void (*get_attr_type_list)(TF_OpKernelConstruction* ctx, const char* attr_name, TFDataTypeEnum* out_vals, int max_vals, TF_Status* out_status);
        void (*get_attr_int32_list)(TF_OpKernelConstruction* ctx, const char* attr_name, int32_t* out_vals, int max_vals, TF_Status* out_status);
        void (*get_attr_int64_list)(TF_OpKernelConstruction* ctx, const char* attr_name, int64_t* out_vals, int max_vals, TF_Status* out_status);
        void (*get_attr_float_list)(TF_OpKernelConstruction* ctx, const char* attr_name, float* out_vals, int max_vals, TF_Status* out_status);
        void (*get_attr_bool_list)(TF_OpKernelConstruction* ctx, const char* attr_name, TF_Bool* out_vals, int max_vals, TF_Status* out_status);
        void (*get_attr_string_list)(TF_OpKernelConstruction* ctx, const char* attr_name, char** out_values, size_t* out_lengths, int max_values, void* storage, size_t storage_size, TF_Status* out_status);
        void (*get_attr_tensor_list)(TF_OpKernelConstruction* ctx, const char* attr_name, TF_Tensor** out_vals, int max_values, TF_Status* out_status);
        void (*get_attr_function)(TF_OpKernelConstruction* ctx, const char* attr_name, TF_Buffer* buffer, TF_Status* out_status);
        void (*has_attr)(TF_OpKernelConstruction* ctx, const char* attr_name, bool* out_has_attr, TF_Status* out_status);
        void (*get_name)(TF_OpKernelConstruction* ctx, TF_String* out_name);
    } TF_OpKernelConstructionOps;
    #define TF_OP_KERNEL_CONSTRUCTION_STRUCT_SIZE TF_OFFSET_OF_END(TF_OpKernelConstructionOps, get_name)
    TF_CAPI_EXPORT void create_op_kernel_construction(TF_OpKernelConstructionOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_op_kernel_construction(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_KERNEL_CONSTRUCTION_H_
''',

    'include/c/extern/kernel/context.h': '''#ifndef TENSORFLOW_C_EXTERN_KERNEL_CONTEXT_H_
#define TENSORFLOW_C_EXTERN_KERNEL_CONTEXT_H_

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

    typedef struct TF_OpKernelContext { void* plugin_data; } TF_OpKernelContext;

    typedef struct {
        size_t struct_size;
        void* priv;
        int start;
        int stop;
        TF_Status* out_status;
    } TF_InputRange_Args;
    #define TF_InputRange_Args_STRUCT_SIZE TF_OFFSET_OF_END(TF_InputRange_Args, out_status)

    // TF_OpKernelContextOps
    typedef struct TF_OpKernelContextOps {
        size_t struct_size;
        void (*num_inputs)(TF_OpKernelContext* ctx, int* out_num);
        void (*num_outputs)(TF_OpKernelContext* ctx, int* out_num);
        void (*get_input)(TF_OpKernelContext* ctx, int i, TF_Tensor** out_tensor, TF_Status* out_status);
        void (*input_range)(TF_OpKernelContext* ctx, const char* name, TF_InputRange_Args* out_args);
        void (*input_datatype)(TF_OpKernelContext* ctx, int index, TFDataTypeEnum* out_type);
        void (*set_output)(TF_OpKernelContext* ctx, int i, const TF_Tensor* tensor, TF_Status* out_status);
        void (*get_mutable_output)(TF_OpKernelContext* ctx, int i, TF_Tensor** out_tensor, TF_Status* out_status);
        void (*get_serialized_function_def_library)(TF_OpKernelContext* ctx, TF_Buffer* serialized_function_def_library, TF_Status* out_status);
        void (*get_serialized_config_proto)(TF_OpKernelContext* ctx, TF_Buffer* serialized_config_proto, TF_Status* out_status);
        void (*get_serialized_resource_handle_proto)(TF_OpKernelContext* ctx, int i, TF_Buffer* serialized_resource_handle_proto, TF_Status* out_status);
        void (*failure)(TF_OpKernelContext* ctx, TF_Status* out_status);
        void (*expected_output_datatype)(TF_OpKernelContext* ctx, int i, TFDataTypeEnum* out_type);
        void (*is_host_memory_input)(TF_OpKernelContext* ctx, int i, bool* out_is_host, TF_Status* out_status);
        void (*is_host_memory_output)(TF_OpKernelContext* ctx, int i, bool* out_is_host, TF_Status* out_status);
        void (*step_id)(TF_OpKernelContext* ctx, int64_t* out_id);
        void (*get_frame_id)(TF_OpKernelContext* ctx, uint64_t* out_id);
        void (*get_iter_id)(TF_OpKernelContext* ctx, int64_t* out_id);
        void (*get_step_id)(TF_OpKernelContext* ctx, int64_t* out_id);
        void (*get_device_id)(TF_OpKernelContext* ctx, int* out_id);
        void (*get_device_name)(TF_OpKernelContext* ctx, TF_String* out_name);
        void (*get_graph_def_version)(TF_OpKernelContext* ctx, int* out_version);
        void (*get_op_kernel_name)(TF_OpKernelContext* ctx, TF_String* out_name);
        void (*get_resource_mgr_default_container_name)(TF_OpKernelContext* ctx, TF_String* out_name);
        void (*get_op_kernel_requested_input)(TF_OpKernelContext* ctx, size_t index, TF_String* out_name);
        void (*allocate_output)(TF_OpKernelContext* context, int index, TFDataTypeEnum dtype, const int64_t* dims, int num_dims, size_t len, TF_Tensor** out_tensor, TF_Status* out_status);
        void (*forward_input_or_allocate_output)(TF_OpKernelContext* context, const int* candidate_input_indices, int num_candidate_input_indices, int output_index, const int64_t* output_dims, int output_num_dims, int* out_forwarded_input, TF_Tensor** out_tensor, TF_Status* out_status);
        void (*allocate_temp)(TF_OpKernelContext* context, TFDataTypeEnum dtype, const int64_t* dims, int num_dims, void* alloc_attrs, TF_Tensor** out_tensor, TF_Status* out_status);
        void (*inc_num_deferred_ops)(TF_OpKernelContext* context);
        void (*dec_num_deferred_ops)(TF_OpKernelContext* context);
    } TF_OpKernelContextOps;
    #define TF_OP_KERNEL_CONTEXT_STRUCT_SIZE TF_OFFSET_OF_END(TF_OpKernelContextOps, dec_num_deferred_ops)
    TF_CAPI_EXPORT void create_op_kernel_context(TF_OpKernelContextOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_op_kernel_context(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_KERNEL_CONTEXT_H_
''',

    'include/c/extern/kernel/kernel.h': '''#ifndef TENSORFLOW_C_EXTERN_KERNEL_KERNEL_H_
#define TENSORFLOW_C_EXTERN_KERNEL_KERNEL_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include "include/c/extern/kernel/builder.h"
#include "include/c/extern/kernel/construction.h"
#include "include/c/extern/kernel/context.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Kernel {
        void* plugin_data;
        const TF_KernelBuilderOps* builder_ops;
        const TF_OpKernelConstructionOps* construction_ops;
        const TF_OpKernelContextOps* context_ops;
    } TF_Kernel;

    typedef struct TF_KernelOps {
        size_t struct_size;
        void (*destroy)(TF_Kernel* kernel);
        void (*get_name)(TF_Kernel* kernel, TF_String* out_name);
    } TF_KernelOps;

    #define TF_KERNEL_STRUCT_SIZE TF_OFFSET_OF_END(TF_KernelOps, get_name)

    TF_CAPI_EXPORT void create_kernel(TF_KernelOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_kernel(void* plugin_context);

    static inline void init_kernel(TF_KernelOps** ops, TF_Kernel* kernel, TF_Status* out_status) {
        create_kernel(ops, &kernel->plugin_data, out_status);

        TF_KernelBuilderOps* builder_ops = NULL;
        create_kernel_builder(&builder_ops, &kernel->plugin_data, "", "", NULL, NULL, NULL, out_status);
        kernel->builder_ops = builder_ops;

        TF_OpKernelConstructionOps* construction_ops = NULL;
        create_op_kernel_construction(&construction_ops, &kernel->plugin_data, out_status);
        kernel->construction_ops = construction_ops;

        TF_OpKernelContextOps* context_ops = NULL;
        create_op_kernel_context(&context_ops, &kernel->plugin_data, out_status);
        kernel->context_ops = context_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_KERNEL_KERNEL_H_
'''
}

for filepath, content in files.items():
    with open(filepath, 'w') as f:
        f.write(content)
