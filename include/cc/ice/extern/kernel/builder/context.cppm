// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/context.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/context.h"

export module cc_ice_builder_kernel:context;

import std;

export namespace ice::builder {

class TF_OpKernelContextOps
{
public:
    static TF_OpKernelContextOps* create(void* ctx) noexcept
    {
        return static_cast<TF_OpKernelContextOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OpKernelContextOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_OpKernelContextOps*>(handle->plugin_data);
    }

    virtual ~TF_OpKernelContextOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> num_inputs(int* out_num) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> num_outputs(int* out_num) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_input(int i, TF_Tensor** out_tensor) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    input_range(const char* name, TF_InputRange_Args* out_args) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    input_datatype(int index, TFDataTypeEnum* out_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_output(int i, const ice::sonic::TF_TensorOps& tensor) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_mutable_output(int i, TF_Tensor** out_tensor) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_serialized_function_def_library(
        const ice::sonic::TF_BufferOps& serialized_function_def_library
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_serialized_config_proto(
        const ice::sonic::TF_BufferOps& serialized_config_proto
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_serialized_resource_handle_proto(
        int i,
        const ice::sonic::TF_BufferOps& serialized_resource_handle_proto
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> failure() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    expected_output_datatype(int i, TFDataTypeEnum* out_type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_host_memory_input(int i, _Bool* out_is_host) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_host_memory_output(int i, _Bool* out_is_host) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> step_id(int64_t* out_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_frame_id(uint64_t* out_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_iter_id(int64_t* out_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_step_id(int64_t* out_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_device_id(int* out_id) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_device_name(const ice::sonic::TF_StringOps& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_graph_def_version(int* out_version) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_op_kernel_name(const ice::sonic::TF_StringOps& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_resource_mgr_default_container_name(const ice::sonic::TF_StringOps& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_op_kernel_requested_input(
        size_t index,
        const ice::sonic::TF_StringOps& out_name
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> allocate_output(
        int index,
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        size_t len,
        TF_Tensor** out_tensor
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> forward_input_or_allocate_output(
        const int* candidate_input_indices,
        int num_candidate_input_indices,
        int output_index,
        const int64_t* output_dims,
        int output_num_dims,
        int* out_forwarded_input,
        TF_Tensor** out_tensor
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> allocate_temp(
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        void* alloc_attrs,
        TF_Tensor** out_tensor
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> inc_num_deferred_ops() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> dec_num_deferred_ops() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> assign_variable(
        int input_index,
        int value_index,
        _Bool validate_shape,
        TF_CopyTensorFunc copy_func
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> assign_ref_variable(
        int input_ref_index,
        int output_ref_index,
        int value_index,
        _Bool use_locking,
        _Bool validate_shape,
        TF_CopyTensorFunc copy_func
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> assign_update_variable(
        int input_index,
        int value_index,
        int op,
        int is_variant_type,
        TF_CopyTensorFunc copy_func,
        TF_UpdateTensorFunc update_func
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> temporary_variable(
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        const ice::sonic::TF_StringOps& var_name,
        TF_PluginAllocatorFunc plugin_allocator
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    destroy_temporary_variable(int index, const ice::sonic::TF_StringOps& var_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    maybe_lock_variable_input_mutexes_in_order(
        _Bool do_lock,
        _Bool sparse,
        const int* inputs,
        size_t len,
        TF_CopyTensorFunc copy_func,
        TF_VariableInputLockHolder** out_lock_holder
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    release_variable_input_lock_holder(TF_VariableInputLockHolder* lock_holder) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_input_tensor_from_variable(
        int input,
        _Bool lock_held,
        _Bool is_variant_type,
        _Bool sparse,
        TF_CopyTensorFunc copy_func,
        TF_Tensor** out_tensor
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    forward_ref_input_to_ref_output(int32_t input_index, int32_t output_index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    is_ref_input(int i, _Bool* out_is_ref) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_input_by_name(const char* input_name, TF_Tensor** out_tensor) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    add_n_variant(TF_BinaryAddFunc binary_add_func) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    zeros_like_variant(TF_ZerosLikeFunc zeros_like_func) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_stream(void** out_stream) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    run_async_done_callback(void* done_callback) noexcept = 0;

    static TF_OpKernelContextOps* get_generic_vtable()
    {
        static TF_OpKernelContextOps vtable = {
            .struct_size = TF_OPKERNELCONTEXT_STRUCT_SIZE,
            .num_inputs =
                [](TF_OpKernelContext* ctx, int* out_num) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->num_inputs(out_num);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .num_outputs =
                [](TF_OpKernelContext* ctx, int* out_num) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->num_outputs(out_num);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_input =
                [](TF_OpKernelContext* ctx,
                   int i,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_input(i, out_tensor);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .input_range =
                [](TF_OpKernelContext* ctx, const char* name, TF_InputRange_Args* out_args) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->input_range(name, out_args);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .input_datatype =
                [](TF_OpKernelContext* ctx, int index, TFDataTypeEnum* out_type) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->input_datatype(index, out_type);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_output =
                [](TF_OpKernelContext* ctx,
                   int i,
                   const TF_Tensor* tensor,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->set_output(i, ice::sonic::TF_TensorOps::wrap(tensor));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_mutable_output =
                [](TF_OpKernelContext* ctx,
                   int i,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_mutable_output(i, out_tensor);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_serialized_function_def_library =
                [](TF_OpKernelContext* ctx,
                   TF_Buffer* serialized_function_def_library,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_serialized_function_def_library(
                    ice::sonic::TF_BufferOps::wrap(serialized_function_def_library)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_serialized_config_proto =
                [](TF_OpKernelContext* ctx,
                   TF_Buffer* serialized_config_proto,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_serialized_config_proto(
                    ice::sonic::TF_BufferOps::wrap(serialized_config_proto)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_serialized_resource_handle_proto =
                [](TF_OpKernelContext* ctx,
                   int i,
                   TF_Buffer* serialized_resource_handle_proto,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_serialized_resource_handle_proto(
                    i,
                    ice::sonic::TF_BufferOps::wrap(serialized_resource_handle_proto)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .failure =
                [](TF_OpKernelContext* ctx, TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->failure();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .expected_output_datatype =
                [](TF_OpKernelContext* ctx, int i, TFDataTypeEnum* out_type) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->expected_output_datatype(i, out_type);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_host_memory_input =
                [](TF_OpKernelContext* ctx,
                   int i,
                   _Bool* out_is_host,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->is_host_memory_input(i, out_is_host);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .is_host_memory_output =
                [](TF_OpKernelContext* ctx,
                   int i,
                   _Bool* out_is_host,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->is_host_memory_output(i, out_is_host);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .step_id =
                [](TF_OpKernelContext* ctx, int64_t* out_id) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->step_id(out_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_frame_id =
                [](TF_OpKernelContext* ctx, uint64_t* out_id) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_frame_id(out_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_iter_id =
                [](TF_OpKernelContext* ctx, int64_t* out_id) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_iter_id(out_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_step_id =
                [](TF_OpKernelContext* ctx, int64_t* out_id) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_step_id(out_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_device_id =
                [](TF_OpKernelContext* ctx, int* out_id) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_device_id(out_id);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_device_name =
                [](TF_OpKernelContext* ctx, TF_String* out_name) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_device_name(ice::sonic::TF_StringOps::wrap(out_name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_graph_def_version =
                [](TF_OpKernelContext* ctx, int* out_version) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_graph_def_version(out_version);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_op_kernel_name =
                [](TF_OpKernelContext* ctx, TF_String* out_name) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_op_kernel_name(ice::sonic::TF_StringOps::wrap(out_name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_resource_mgr_default_container_name =
                [](TF_OpKernelContext* ctx, TF_String* out_name) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_resource_mgr_default_container_name(
                    ice::sonic::TF_StringOps::wrap(out_name)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_op_kernel_requested_input =
                [](TF_OpKernelContext* ctx, size_t index, TF_String* out_name) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_op_kernel_requested_input(
                    index,
                    ice::sonic::TF_StringOps::wrap(out_name)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .allocate_output =
                [](TF_OpKernelContext* context,
                   int index,
                   TFDataTypeEnum dtype,
                   const int64_t* dims,
                   int num_dims,
                   size_t len,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(context);
                auto res = self->allocate_output(index, dtype, dims, num_dims, len, out_tensor);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .forward_input_or_allocate_output =
                [](TF_OpKernelContext* context,
                   const int* candidate_input_indices,
                   int num_candidate_input_indices,
                   int output_index,
                   const int64_t* output_dims,
                   int output_num_dims,
                   int* out_forwarded_input,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(context);
                auto res = self->forward_input_or_allocate_output(
                    candidate_input_indices,
                    num_candidate_input_indices,
                    output_index,
                    output_dims,
                    output_num_dims,
                    out_forwarded_input,
                    out_tensor
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .allocate_temp =
                [](TF_OpKernelContext* context,
                   TFDataTypeEnum dtype,
                   const int64_t* dims,
                   int num_dims,
                   void* alloc_attrs,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(context);
                auto res = self->allocate_temp(dtype, dims, num_dims, alloc_attrs, out_tensor);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .inc_num_deferred_ops =
                [](TF_OpKernelContext* context) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(context);
                auto res = self->inc_num_deferred_ops();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .dec_num_deferred_ops =
                [](TF_OpKernelContext* context) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(context);
                auto res = self->dec_num_deferred_ops();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .assign_variable =
                [](TF_OpKernelContext* ctx,
                   int input_index,
                   int value_index,
                   _Bool validate_shape,
                   TF_CopyTensorFunc copy_func,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res =
                    self->assign_variable(input_index, value_index, validate_shape, copy_func);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .assign_ref_variable =
                [](TF_OpKernelContext* ctx,
                   int input_ref_index,
                   int output_ref_index,
                   int value_index,
                   _Bool use_locking,
                   _Bool validate_shape,
                   TF_CopyTensorFunc copy_func,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->assign_ref_variable(
                    input_ref_index,
                    output_ref_index,
                    value_index,
                    use_locking,
                    validate_shape,
                    copy_func
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .assign_update_variable =
                [](TF_OpKernelContext* ctx,
                   int input_index,
                   int value_index,
                   int op,
                   int is_variant_type,
                   TF_CopyTensorFunc copy_func,
                   TF_UpdateTensorFunc update_func,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->assign_update_variable(
                    input_index,
                    value_index,
                    op,
                    is_variant_type,
                    copy_func,
                    update_func
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .temporary_variable =
                [](TF_OpKernelContext* ctx,
                   TFDataTypeEnum dtype,
                   const int64_t* dims,
                   int num_dims,
                   const TF_String* var_name,
                   TF_PluginAllocatorFunc plugin_allocator,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->temporary_variable(
                    dtype,
                    dims,
                    num_dims,
                    ice::sonic::TF_StringOps::wrap(var_name),
                    plugin_allocator
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .destroy_temporary_variable =
                [](TF_OpKernelContext* ctx,
                   int index,
                   const TF_String* var_name,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->destroy_temporary_variable(
                    index,
                    ice::sonic::TF_StringOps::wrap(var_name)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .maybe_lock_variable_input_mutexes_in_order =
                [](TF_OpKernelContext* ctx,
                   _Bool do_lock,
                   _Bool sparse,
                   const int* inputs,
                   size_t len,
                   TF_CopyTensorFunc copy_func,
                   TF_VariableInputLockHolder** out_lock_holder,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->maybe_lock_variable_input_mutexes_in_order(
                    do_lock,
                    sparse,
                    inputs,
                    len,
                    copy_func,
                    out_lock_holder
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .release_variable_input_lock_holder =
                [](TF_OpKernelContext* ctx, TF_VariableInputLockHolder* lock_holder) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->release_variable_input_lock_holder(lock_holder);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_input_tensor_from_variable =
                [](TF_OpKernelContext* ctx,
                   int input,
                   _Bool lock_held,
                   _Bool is_variant_type,
                   _Bool sparse,
                   TF_CopyTensorFunc copy_func,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_input_tensor_from_variable(
                    input,
                    lock_held,
                    is_variant_type,
                    sparse,
                    copy_func,
                    out_tensor
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .forward_ref_input_to_ref_output =
                [](TF_OpKernelContext* ctx, int32_t input_index, int32_t output_index) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->forward_ref_input_to_ref_output(input_index, output_index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .is_ref_input =
                [](TF_OpKernelContext* ctx,
                   int i,
                   _Bool* out_is_ref,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->is_ref_input(i, out_is_ref);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_input_by_name =
                [](TF_OpKernelContext* ctx,
                   const char* input_name,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_input_by_name(input_name, out_tensor);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .add_n_variant =
                [](TF_OpKernelContext* ctx,
                   TF_BinaryAddFunc binary_add_func,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->add_n_variant(binary_add_func);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .zeros_like_variant =
                [](TF_OpKernelContext* ctx,
                   TF_ZerosLikeFunc zeros_like_func,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->zeros_like_variant(zeros_like_func);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stream =
                [](TF_OpKernelContext* ctx, void** out_stream, TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->get_stream(out_stream);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .run_async_done_callback =
                [](TF_OpKernelContext* ctx, void* done_callback) noexcept
            {
                auto* self = TF_OpKernelContextOps::create(ctx);
                auto res = self->run_async_done_callback(done_callback);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
