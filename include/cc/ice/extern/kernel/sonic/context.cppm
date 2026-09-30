// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/context.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/context.h"

export module cc_ice_extern_kernel_sonic:context;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_OpKernelContextOps :
    public ice::sonic::Runtime<TF_OpKernelContextOps, TF_OpKernelContextOps>
{
public:
    explicit TF_OpKernelContextOps(TF_OpKernelContextOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "kernel";

    [[nodiscard]] std::expected<void, ice::Status> num_inputs(int* out_num) noexcept
    {
        ice::Status status;
        m_ops->num_inputs(get_handle(), out_num, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> num_outputs(int* out_num) noexcept
    {
        ice::Status status;
        m_ops->num_outputs(get_handle(), out_num, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_input(int i, TF_Tensor** out_tensor) noexcept
    {
        ice::Status status;
        m_ops->get_input(get_handle(), i, out_tensor, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    input_range(const ice::sonic::String& name, TF_InputRange_Args* out_args) noexcept
    {
        ice::Status status;
        m_ops->input_range(get_handle(), name.get_handle(), out_args, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    input_datatype(int index, TFDataTypeEnum* out_type) noexcept
    {
        ice::Status status;
        m_ops->input_datatype(get_handle(), index, out_type, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_output(int i, const ice::sonic::TF_TensorOps& tensor) noexcept
    {
        ice::Status status;
        m_ops->set_output(get_handle(), i, tensor.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_mutable_output(int i, TF_Tensor** out_tensor) noexcept
    {
        ice::Status status;
        m_ops->get_mutable_output(get_handle(), i, out_tensor, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_serialized_function_def_library(
        const ice::sonic::TF_BufferOps& serialized_function_def_library
    ) noexcept
    {
        ice::Status status;
        m_ops->get_serialized_function_def_library(
            get_handle(),
            serialized_function_def_library.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_serialized_config_proto(const ice::sonic::TF_BufferOps& serialized_config_proto) noexcept
    {
        ice::Status status;
        m_ops->get_serialized_config_proto(
            get_handle(),
            serialized_config_proto.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_serialized_resource_handle_proto(
        int i,
        const ice::sonic::TF_BufferOps& serialized_resource_handle_proto
    ) noexcept
    {
        ice::Status status;
        m_ops->get_serialized_resource_handle_proto(
            get_handle(),
            i,
            serialized_resource_handle_proto.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> failure() noexcept
    {
        ice::Status status;
        m_ops->failure(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    expected_output_datatype(int i, TFDataTypeEnum* out_type) noexcept
    {
        ice::Status status;
        m_ops->expected_output_datatype(get_handle(), i, out_type, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    is_host_memory_input(int i, _Bool* out_is_host) noexcept
    {
        ice::Status status;
        m_ops->is_host_memory_input(get_handle(), i, out_is_host, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    is_host_memory_output(int i, _Bool* out_is_host) noexcept
    {
        ice::Status status;
        m_ops->is_host_memory_output(get_handle(), i, out_is_host, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> step_id(int64_t* out_id) noexcept
    {
        ice::Status status;
        m_ops->step_id(get_handle(), out_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_frame_id(uint64_t* out_id) noexcept
    {
        ice::Status status;
        m_ops->get_frame_id(get_handle(), out_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_iter_id(int64_t* out_id) noexcept
    {
        ice::Status status;
        m_ops->get_iter_id(get_handle(), out_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_step_id(int64_t* out_id) noexcept
    {
        ice::Status status;
        m_ops->get_step_id(get_handle(), out_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_device_id(int* out_id) noexcept
    {
        ice::Status status;
        m_ops->get_device_id(get_handle(), out_id, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_device_name(const ice::sonic::String& out_name) noexcept
    {
        ice::Status status;
        m_ops->get_device_name(get_handle(), out_name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_graph_def_version(int* out_version) noexcept
    {
        ice::Status status;
        m_ops->get_graph_def_version(get_handle(), out_version, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_op_kernel_name(const ice::sonic::String& out_name) noexcept
    {
        ice::Status status;
        m_ops->get_op_kernel_name(get_handle(), out_name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_resource_mgr_default_container_name(const ice::sonic::String& out_name) noexcept
    {
        ice::Status status;
        m_ops->get_resource_mgr_default_container_name(
            get_handle(),
            out_name.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_op_kernel_requested_input(size_t index, const ice::sonic::String& out_name) noexcept
    {
        ice::Status status;
        m_ops->get_op_kernel_requested_input(
            get_handle(),
            index,
            out_name.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> allocate_output(
        int index,
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        size_t len,
        TF_Tensor** out_tensor
    ) noexcept
    {
        ice::Status status;
        m_ops->allocate_output(
            get_handle(),
            index,
            dtype,
            dims,
            num_dims,
            len,
            out_tensor,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> forward_input_or_allocate_output(
        const int* candidate_input_indices,
        int num_candidate_input_indices,
        int output_index,
        const int64_t* output_dims,
        int output_num_dims,
        int* out_forwarded_input,
        TF_Tensor** out_tensor
    ) noexcept
    {
        ice::Status status;
        m_ops->forward_input_or_allocate_output(
            get_handle(),
            candidate_input_indices,
            num_candidate_input_indices,
            output_index,
            output_dims,
            output_num_dims,
            out_forwarded_input,
            out_tensor,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> allocate_temp(
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        void* alloc_attrs,
        TF_Tensor** out_tensor
    ) noexcept
    {
        ice::Status status;
        m_ops->allocate_temp(
            get_handle(),
            dtype,
            dims,
            num_dims,
            alloc_attrs,
            out_tensor,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> inc_num_deferred_ops() noexcept
    {
        ice::Status status;
        m_ops->inc_num_deferred_ops(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> dec_num_deferred_ops() noexcept
    {
        ice::Status status;
        m_ops->dec_num_deferred_ops(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> assign_variable(
        int input_index,
        int value_index,
        _Bool validate_shape,
        TF_CopyTensorFunc copy_func
    ) noexcept
    {
        ice::Status status;
        m_ops->assign_variable(
            get_handle(),
            input_index,
            value_index,
            validate_shape,
            copy_func,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> assign_ref_variable(
        int input_ref_index,
        int output_ref_index,
        int value_index,
        _Bool use_locking,
        _Bool validate_shape,
        TF_CopyTensorFunc copy_func
    ) noexcept
    {
        ice::Status status;
        m_ops->assign_ref_variable(
            get_handle(),
            input_ref_index,
            output_ref_index,
            value_index,
            use_locking,
            validate_shape,
            copy_func,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> assign_update_variable(
        int input_index,
        int value_index,
        int op,
        int is_variant_type,
        TF_CopyTensorFunc copy_func,
        TF_UpdateTensorFunc update_func
    ) noexcept
    {
        ice::Status status;
        m_ops->assign_update_variable(
            get_handle(),
            input_index,
            value_index,
            op,
            is_variant_type,
            copy_func,
            update_func,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> temporary_variable(
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        const ice::sonic::String& var_name,
        TF_PluginAllocatorFunc plugin_allocator
    ) noexcept
    {
        ice::Status status;
        m_ops->temporary_variable(
            get_handle(),
            dtype,
            dims,
            num_dims,
            var_name.get_handle(),
            plugin_allocator,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    destroy_temporary_variable(int index, const ice::sonic::String& var_name) noexcept
    {
        ice::Status status;
        m_ops->destroy_temporary_variable(
            get_handle(),
            index,
            var_name.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> maybe_lock_variable_input_mutexes_in_order(
        _Bool do_lock,
        _Bool sparse,
        const int* inputs,
        size_t len,
        TF_CopyTensorFunc copy_func,
        TF_VariableInputLockHolder** out_lock_holder
    ) noexcept
    {
        ice::Status status;
        m_ops->maybe_lock_variable_input_mutexes_in_order(
            get_handle(),
            do_lock,
            sparse,
            inputs,
            len,
            copy_func,
            out_lock_holder,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    release_variable_input_lock_holder(TF_VariableInputLockHolder* lock_holder) noexcept
    {
        ice::Status status;
        m_ops->release_variable_input_lock_holder(get_handle(), lock_holder, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_input_tensor_from_variable(
        int input,
        _Bool lock_held,
        _Bool is_variant_type,
        _Bool sparse,
        TF_CopyTensorFunc copy_func,
        TF_Tensor** out_tensor
    ) noexcept
    {
        ice::Status status;
        m_ops->get_input_tensor_from_variable(
            get_handle(),
            input,
            lock_held,
            is_variant_type,
            sparse,
            copy_func,
            out_tensor,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    forward_ref_input_to_ref_output(int32_t input_index, int32_t output_index) noexcept
    {
        ice::Status status;
        m_ops->forward_ref_input_to_ref_output(
            get_handle(),
            input_index,
            output_index,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_ref_input(int i, _Bool* out_is_ref) noexcept
    {
        ice::Status status;
        m_ops->is_ref_input(get_handle(), i, out_is_ref, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_input_by_name(const ice::sonic::String& input_name, TF_Tensor** out_tensor) noexcept
    {
        ice::Status status;
        m_ops->get_input_by_name(
            get_handle(),
            input_name.get_handle(),
            out_tensor,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    add_n_variant(TF_BinaryAddFunc binary_add_func) noexcept
    {
        ice::Status status;
        m_ops->add_n_variant(get_handle(), binary_add_func, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    zeros_like_variant(TF_ZerosLikeFunc zeros_like_func) noexcept
    {
        ice::Status status;
        m_ops->zeros_like_variant(get_handle(), zeros_like_func, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_stream(TF_Stream** out_stream) noexcept
    {
        ice::Status status;
        m_ops->get_stream(get_handle(), out_stream, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    run_async_done_callback(void* done_callback) noexcept
    {
        ice::Status status;
        m_ops->run_async_done_callback(get_handle(), done_callback, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_random_generator(const ice::sonic::TF_RandomGeneratorOps& out_generator) noexcept
    {
        ice::Status status;
        m_ops->get_random_generator(get_handle(), out_generator.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    sonic::String get_name() const noexcept
    {
        sonic::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
