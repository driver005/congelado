#ifndef TENSORFLOW_C_EXTERN_KERNEL_BUILDER_H_
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
