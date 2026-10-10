// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/context.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/context.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_kernel_sonic:context;

import std;
import cc_ice_extern_random_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_OpKernelContextOps :
    public ice::sonic::Runtime<::TF_OpKernelContextOps, ::TF_OpKernelContext>
{
public:
    TF_OpKernelContextOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_OpKernelContextOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_OpKernelContext* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_OpKernelContextOps(const ::TF_OpKernelContextOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_OpKernelContextOps(const ::TF_OpKernelContextOps* ops, ::TF_OpKernelContext* handle) noexcept
        :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void num_inputs(int* out_num) const noexcept
    {
        m_ops->num_inputs(get_handle(), out_num);
    }

    void num_outputs(int* out_num) const noexcept
    {
        m_ops->num_outputs(get_handle(), out_num);
    }

    void get_input(
        int i,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_input(get_handle(), i, out_tensor, out_status.get_handle());
    }

    void input_range(const ice::sonic::String& name, TF_InputRange_Args* out_args) const noexcept
    {
        m_ops->input_range(get_handle(), name.get_handle(), out_args);
    }

    void input_datatype(int index, TFDataTypeEnum* out_type) const noexcept
    {
        m_ops->input_datatype(get_handle(), index, out_type);
    }

    void set_output(
        int i,
        const ice::sonic::TF_TensorOps& tensor,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_output(get_handle(), i, tensor.get_handle(), out_status.get_handle());
    }

    void get_mutable_output(
        int i,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_mutable_output(get_handle(), i, out_tensor, out_status.get_handle());
    }

    void get_serialized_function_def_library(
        const ice::sonic::TF_BufferOps& serialized_function_def_library,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_serialized_function_def_library(
            get_handle(),
            serialized_function_def_library.get_handle(),
            out_status.get_handle()
        );
    }

    void get_serialized_config_proto(
        const ice::sonic::TF_BufferOps& serialized_config_proto,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_serialized_config_proto(
            get_handle(),
            serialized_config_proto.get_handle(),
            out_status.get_handle()
        );
    }

    void get_serialized_resource_handle_proto(
        int i,
        const ice::sonic::TF_BufferOps& serialized_resource_handle_proto,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_serialized_resource_handle_proto(
            get_handle(),
            i,
            serialized_resource_handle_proto.get_handle(),
            out_status.get_handle()
        );
    }

    void failure(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->failure(get_handle(), out_status.get_handle());
    }

    void expected_output_datatype(int i, TFDataTypeEnum* out_type) const noexcept
    {
        m_ops->expected_output_datatype(get_handle(), i, out_type);
    }

    void is_host_memory_input(
        int i,
        _Bool* out_is_host,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->is_host_memory_input(get_handle(), i, out_is_host, out_status.get_handle());
    }

    void is_host_memory_output(
        int i,
        _Bool* out_is_host,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->is_host_memory_output(get_handle(), i, out_is_host, out_status.get_handle());
    }

    void step_id(int64_t* out_id) const noexcept
    {
        m_ops->step_id(get_handle(), out_id);
    }

    void get_frame_id(uint64_t* out_id) const noexcept
    {
        m_ops->get_frame_id(get_handle(), out_id);
    }

    void get_iter_id(int64_t* out_id) const noexcept
    {
        m_ops->get_iter_id(get_handle(), out_id);
    }

    void get_step_id(int64_t* out_id) const noexcept
    {
        m_ops->get_step_id(get_handle(), out_id);
    }

    void get_device_id(int* out_id) const noexcept
    {
        m_ops->get_device_id(get_handle(), out_id);
    }

    void get_device_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_device_name(get_handle(), out_name.get_handle());
    }

    void get_graph_def_version(int* out_version) const noexcept
    {
        m_ops->get_graph_def_version(get_handle(), out_version);
    }

    void get_op_kernel_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_op_kernel_name(get_handle(), out_name.get_handle());
    }

    void get_resource_mgr_default_container_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_resource_mgr_default_container_name(get_handle(), out_name.get_handle());
    }

    void get_op_kernel_requested_input(
        size_t index,
        const ice::sonic::String& out_name
    ) const noexcept
    {
        m_ops->get_op_kernel_requested_input(get_handle(), index, out_name.get_handle());
    }

    void allocate_output(
        int index,
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        size_t len,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->allocate_output(
            get_handle(),
            index,
            dtype,
            dims,
            num_dims,
            len,
            out_tensor,
            out_status.get_handle()
        );
    }

    void forward_input_or_allocate_output(
        const int* candidate_input_indices,
        int num_candidate_input_indices,
        int output_index,
        const int64_t* output_dims,
        int output_num_dims,
        int* out_forwarded_input,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->forward_input_or_allocate_output(
            get_handle(),
            candidate_input_indices,
            num_candidate_input_indices,
            output_index,
            output_dims,
            output_num_dims,
            out_forwarded_input,
            out_tensor,
            out_status.get_handle()
        );
    }

    void allocate_temp(
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        void* alloc_attrs,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->allocate_temp(
            get_handle(),
            dtype,
            dims,
            num_dims,
            alloc_attrs,
            out_tensor,
            out_status.get_handle()
        );
    }

    void inc_num_deferred_ops() const noexcept
    {
        m_ops->inc_num_deferred_ops(get_handle());
    }

    void dec_num_deferred_ops() const noexcept
    {
        m_ops->dec_num_deferred_ops(get_handle());
    }

    void assign_variable(
        int input_index,
        int value_index,
        _Bool validate_shape,
        TF_CopyTensorFunc copy_func,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->assign_variable(
            get_handle(),
            input_index,
            value_index,
            validate_shape,
            copy_func,
            out_status.get_handle()
        );
    }

    void assign_ref_variable(
        int input_ref_index,
        int output_ref_index,
        int value_index,
        _Bool use_locking,
        _Bool validate_shape,
        TF_CopyTensorFunc copy_func,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->assign_ref_variable(
            get_handle(),
            input_ref_index,
            output_ref_index,
            value_index,
            use_locking,
            validate_shape,
            copy_func,
            out_status.get_handle()
        );
    }

    void assign_update_variable(
        int input_index,
        int value_index,
        int op,
        int is_variant_type,
        TF_CopyTensorFunc copy_func,
        TF_UpdateTensorFunc update_func,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->assign_update_variable(
            get_handle(),
            input_index,
            value_index,
            op,
            is_variant_type,
            copy_func,
            update_func,
            out_status.get_handle()
        );
    }

    void temporary_variable(
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        const ice::sonic::String& var_name,
        TF_PluginAllocatorFunc plugin_allocator,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->temporary_variable(
            get_handle(),
            dtype,
            dims,
            num_dims,
            var_name.get_handle(),
            plugin_allocator,
            out_status.get_handle()
        );
    }

    void destroy_temporary_variable(
        int index,
        const ice::sonic::String& var_name,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->destroy_temporary_variable(
            get_handle(),
            index,
            var_name.get_handle(),
            out_status.get_handle()
        );
    }

    void maybe_lock_variable_input_mutexes_in_order(
        _Bool do_lock,
        _Bool sparse,
        const int* inputs,
        size_t len,
        TF_CopyTensorFunc copy_func,
        TF_VariableInputLockHolder** out_lock_holder,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->maybe_lock_variable_input_mutexes_in_order(
            get_handle(),
            do_lock,
            sparse,
            inputs,
            len,
            copy_func,
            out_lock_holder,
            out_status.get_handle()
        );
    }

    void release_variable_input_lock_holder(TF_VariableInputLockHolder* lock_holder) const noexcept
    {
        m_ops->release_variable_input_lock_holder(get_handle(), lock_holder);
    }

    void get_input_tensor_from_variable(
        int input,
        _Bool lock_held,
        _Bool is_variant_type,
        _Bool sparse,
        TF_CopyTensorFunc copy_func,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_input_tensor_from_variable(
            get_handle(),
            input,
            lock_held,
            is_variant_type,
            sparse,
            copy_func,
            out_tensor,
            out_status.get_handle()
        );
    }

    void forward_ref_input_to_ref_output(int32_t input_index, int32_t output_index) const noexcept
    {
        m_ops->forward_ref_input_to_ref_output(get_handle(), input_index, output_index);
    }

    void is_ref_input(int i, _Bool* out_is_ref, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->is_ref_input(get_handle(), i, out_is_ref, out_status.get_handle());
    }

    void get_input_by_name(
        const ice::sonic::String& input_name,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_input_by_name(
            get_handle(),
            input_name.get_handle(),
            out_tensor,
            out_status.get_handle()
        );
    }

    void add_n_variant(
        TF_BinaryAddFunc binary_add_func,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_n_variant(get_handle(), binary_add_func, out_status.get_handle());
    }

    void zeros_like_variant(
        TF_ZerosLikeFunc zeros_like_func,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->zeros_like_variant(get_handle(), zeros_like_func, out_status.get_handle());
    }

    void get_stream(TF_Stream** out_stream, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->get_stream(get_handle(), out_stream, out_status.get_handle());
    }

    void run_async_done_callback(void* done_callback) const noexcept
    {
        m_ops->run_async_done_callback(get_handle(), done_callback);
    }

    void get_random_generator(
        const ice::sonic::TF_RandomGeneratorOps& out_generator,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_random_generator(
            get_handle(),
            out_generator.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
