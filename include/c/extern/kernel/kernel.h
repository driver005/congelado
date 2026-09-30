#ifndef TENSORFLOW_C_EXTERN_KERNEL_KERNEL_H_
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
        create_kernel_builder(&builder_ops, &kernel->plugin_data, NULL, NULL, NULL, NULL, NULL, out_status);
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
