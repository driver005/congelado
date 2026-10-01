#ifndef TENSORFLOW_C_EXTERN_KERNEL_CONTEXT_H_
#define TENSORFLOW_C_EXTERN_KERNEL_CONTEXT_H_

#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/intern/buffer.h"
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

    typedef struct TF_OpKernelContext
    {
        void* plugin_data;
    } TF_OpKernelContext;

    typedef struct
    {
        size_t struct_size;
        void* priv;
        int start;
        int stop;
        TF_Status* out_status;
    } TF_InputRange_Args;

#define TF_InputRange_Args_STRUCT_SIZE TF_OFFSET_OF_END(TF_InputRange_Args, out_status)

    typedef struct TF_VariableInputLockHolder
    {
        void* plugin_data;
    } TF_VariableInputLockHolder;

    typedef void (*TF_CopyTensorFunc)(TF_OpKernelContext* ctx, TF_Tensor* source, TF_Tensor* dest);
    typedef void (*TF_UpdateTensorFunc)(
        TF_OpKernelContext* ctx,
        TF_Tensor* tensor,
        TF_Tensor* value,
        int op
    );
    typedef void (*TF_PluginAllocatorFunc)(
        TF_OpKernelContext* ctx,
        TF_Tensor* tensor,
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        TF_Status* out_status
    );
    typedef void (*TF_BinaryAddFunc)(
        TF_OpKernelContext* ctx,
        TF_Tensor* a,
        TF_Tensor* b,
        TF_Tensor* out_tensor
    );
    typedef void (*TF_ZerosLikeFunc)(
        TF_OpKernelContext* ctx,
        TF_Tensor* input,
        TF_Tensor* out_tensor
    );

    // TF_OpKernelContextOps
    typedef struct TF_OpKernelContextOps
    {
        size_t struct_size;
        void (*create)(TF_OpKernelContext* out_handle);
        void (*destroy)(TF_OpKernelContext* handle);
        void (*num_inputs)(TF_OpKernelContext* ctx, int* out_num);
        void (*num_outputs)(TF_OpKernelContext* ctx, int* out_num);
        void (*get_input)(
            TF_OpKernelContext* ctx,
            int i,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
        void (*input_range)(
            TF_OpKernelContext* ctx,
            const TF_String* name,
            TF_InputRange_Args* out_args
        );
        void (*input_datatype)(TF_OpKernelContext* ctx, int index, TFDataTypeEnum* out_type);
        void (*set_output)(
            TF_OpKernelContext* ctx,
            int i,
            const TF_Tensor* tensor,
            TF_Status* out_status
        );
        void (*get_mutable_output)(
            TF_OpKernelContext* ctx,
            int i,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
        void (*get_serialized_function_def_library)(
            TF_OpKernelContext* ctx,
            TF_Buffer* serialized_function_def_library,
            TF_Status* out_status
        );
        void (*get_serialized_config_proto)(
            TF_OpKernelContext* ctx,
            TF_Buffer* serialized_config_proto,
            TF_Status* out_status
        );
        void (*get_serialized_resource_handle_proto)(
            TF_OpKernelContext* ctx,
            int i,
            TF_Buffer* serialized_resource_handle_proto,
            TF_Status* out_status
        );
        void (*failure)(TF_OpKernelContext* ctx, TF_Status* out_status);
        void (*expected_output_datatype)(TF_OpKernelContext* ctx, int i, TFDataTypeEnum* out_type);
        void (*is_host_memory_input)(
            TF_OpKernelContext* ctx,
            int i,
            bool* out_is_host,
            TF_Status* out_status
        );
        void (*is_host_memory_output)(
            TF_OpKernelContext* ctx,
            int i,
            bool* out_is_host,
            TF_Status* out_status
        );
        void (*step_id)(TF_OpKernelContext* ctx, int64_t* out_id);
        void (*get_frame_id)(TF_OpKernelContext* ctx, uint64_t* out_id);
        void (*get_iter_id)(TF_OpKernelContext* ctx, int64_t* out_id);
        void (*get_step_id)(TF_OpKernelContext* ctx, int64_t* out_id);
        void (*get_device_id)(TF_OpKernelContext* ctx, int* out_id);
        void (*get_device_name)(TF_OpKernelContext* ctx, TF_String* out_name);
        void (*get_graph_def_version)(TF_OpKernelContext* ctx, int* out_version);
        void (*get_op_kernel_name)(TF_OpKernelContext* ctx, TF_String* out_name);
        void (*get_resource_mgr_default_container_name)(
            TF_OpKernelContext* ctx,
            TF_String* out_name
        );
        void (*get_op_kernel_requested_input)(
            TF_OpKernelContext* ctx,
            size_t index,
            TF_String* out_name
        );
        void (*allocate_output)(
            TF_OpKernelContext* context,
            int index,
            TFDataTypeEnum dtype,
            const int64_t* dims,
            int num_dims,
            size_t len,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
        void (*forward_input_or_allocate_output)(
            TF_OpKernelContext* context,
            const int* candidate_input_indices,
            int num_candidate_input_indices,
            int output_index,
            const int64_t* output_dims,
            int output_num_dims,
            int* out_forwarded_input,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
        void (*allocate_temp)(
            TF_OpKernelContext* context,
            TFDataTypeEnum dtype,
            const int64_t* dims,
            int num_dims,
            void* alloc_attrs,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
        void (*inc_num_deferred_ops)(TF_OpKernelContext* context);
        void (*dec_num_deferred_ops)(TF_OpKernelContext* context);
        void (*assign_variable)(
            TF_OpKernelContext* ctx,
            int input_index,
            int value_index,
            bool validate_shape,
            TF_CopyTensorFunc copy_func,
            TF_Status* out_status
        );
        void (*assign_ref_variable)(
            TF_OpKernelContext* ctx,
            int input_ref_index,
            int output_ref_index,
            int value_index,
            bool use_locking,
            bool validate_shape,
            TF_CopyTensorFunc copy_func,
            TF_Status* out_status
        );
        void (*assign_update_variable)(
            TF_OpKernelContext* ctx,
            int input_index,
            int value_index,
            int op,
            int is_variant_type,
            TF_CopyTensorFunc copy_func,
            TF_UpdateTensorFunc update_func,
            TF_Status* out_status
        );
        void (*temporary_variable)(
            TF_OpKernelContext* ctx,
            TFDataTypeEnum dtype,
            const int64_t* dims,
            int num_dims,
            const TF_String* var_name,
            TF_PluginAllocatorFunc plugin_allocator,
            TF_Status* out_status
        );
        void (*destroy_temporary_variable)(
            TF_OpKernelContext* ctx,
            int index,
            const TF_String* var_name,
            TF_Status* out_status
        );
        void (*maybe_lock_variable_input_mutexes_in_order)(
            TF_OpKernelContext* ctx,
            bool do_lock,
            bool sparse,
            const int* inputs,
            size_t len,
            TF_CopyTensorFunc copy_func,
            TF_VariableInputLockHolder** out_lock_holder,
            TF_Status* out_status
        );
        void (*release_variable_input_lock_holder)(
            TF_OpKernelContext* ctx,
            TF_VariableInputLockHolder* lock_holder
        );
        void (*get_input_tensor_from_variable)(
            TF_OpKernelContext* ctx,
            int input,
            bool lock_held,
            bool is_variant_type,
            bool sparse,
            TF_CopyTensorFunc copy_func,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
        void (*forward_ref_input_to_ref_output)(
            TF_OpKernelContext* ctx,
            int32_t input_index,
            int32_t output_index
        );
        void (*is_ref_input)(
            TF_OpKernelContext* ctx,
            int i,
            bool* out_is_ref,
            TF_Status* out_status
        );
        void (*get_input_by_name)(
            TF_OpKernelContext* ctx,
            const TF_String* input_name,
            TF_Tensor** out_tensor,
            TF_Status* out_status
        );
        void (*add_n_variant)(
            TF_OpKernelContext* ctx,
            TF_BinaryAddFunc binary_add_func,
            TF_Status* out_status
        );
        void (*zeros_like_variant)(
            TF_OpKernelContext* ctx,
            TF_ZerosLikeFunc zeros_like_func,
            TF_Status* out_status
        );
        void (*get_stream)(TF_OpKernelContext* ctx, TF_Stream** out_stream, TF_Status* out_status);
        void (*run_async_done_callback)(TF_OpKernelContext* ctx, void* done_callback);
        void (*get_random_generator)(
            TF_OpKernelContext* ctx,
            TF_RandomGenerator* out_generator,
            TF_Status* out_status
        );
    } TF_OpKernelContextOps;

#define TF_OP_KERNEL_CONTEXT_STRUCT_SIZE                                                           \
    TF_OFFSET_OF_END(TF_OpKernelContextOps, get_random_generator)
    TF_CAPI_EXPORT void create_op_kernel_context(
        TF_OpKernelContextOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_op_kernel_context(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_KERNEL_CONTEXT_H_
