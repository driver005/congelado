// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/context.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/context.h"
#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_kernel_builder:context;

import std;
import cc_ice_extern_random_generator_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_OpKernelContextOps
{
public:
    explicit TF_OpKernelContextOps(
        const ::TF_BufferOps* TF_BufferOps_ops,
        const ::TF_RandomGeneratorOps* TF_RandomGeneratorOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_TensorOps* TF_TensorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_BufferOps_ops = TF_BufferOps_ops;
        m_TF_RandomGeneratorOps_ops = TF_RandomGeneratorOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_TF_TensorOps_ops = TF_TensorOps_ops;
    }

    TF_OpKernelContextOps(const TF_OpKernelContextOps&) = delete;
    TF_OpKernelContextOps& operator=(const TF_OpKernelContextOps&) = delete;

    static TF_OpKernelContextOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_OpKernelContextOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OpKernelContextOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_OpKernelContextOps*>(handle->plugin_data);
    }

    virtual ~TF_OpKernelContextOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void num_inputs(int* out_num) noexcept = 0;
    virtual void num_outputs(int* out_num) noexcept = 0;
    virtual void
    get_input(int i, TF_Tensor** out_tensor, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    input_range(const ice::sonic::String& name, TF_InputRange_Args* out_args) noexcept = 0;
    virtual void input_datatype(int index, TFDataTypeEnum* out_type) noexcept = 0;
    virtual void set_output(
        int i,
        const ice::sonic::TF_TensorOps& tensor,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_mutable_output(
        int i,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_serialized_function_def_library(
        const ice::sonic::TF_BufferOps& serialized_function_def_library,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_serialized_config_proto(
        const ice::sonic::TF_BufferOps& serialized_config_proto,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_serialized_resource_handle_proto(
        int i,
        const ice::sonic::TF_BufferOps& serialized_resource_handle_proto,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void failure(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void expected_output_datatype(int i, TFDataTypeEnum* out_type) noexcept = 0;
    virtual void is_host_memory_input(
        int i,
        _Bool* out_is_host,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void is_host_memory_output(
        int i,
        _Bool* out_is_host,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void step_id(int64_t* out_id) noexcept = 0;
    virtual void get_frame_id(uint64_t* out_id) noexcept = 0;
    virtual void get_iter_id(int64_t* out_id) noexcept = 0;
    virtual void get_step_id(int64_t* out_id) noexcept = 0;
    virtual void get_device_id(int* out_id) noexcept = 0;
    virtual void get_device_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void get_graph_def_version(int* out_version) noexcept = 0;
    virtual void get_op_kernel_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void
    get_resource_mgr_default_container_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void
    get_op_kernel_requested_input(size_t index, const ice::sonic::String& out_name) noexcept = 0;
    virtual void allocate_output(
        int index,
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        size_t len,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void forward_input_or_allocate_output(
        const int* candidate_input_indices,
        int num_candidate_input_indices,
        int output_index,
        const int64_t* output_dims,
        int output_num_dims,
        int* out_forwarded_input,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void allocate_temp(
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        void* alloc_attrs,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void inc_num_deferred_ops() noexcept = 0;
    virtual void dec_num_deferred_ops() noexcept = 0;
    virtual void assign_variable(
        int input_index,
        int value_index,
        _Bool validate_shape,
        TF_CopyTensorFunc copy_func,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void assign_ref_variable(
        int input_ref_index,
        int output_ref_index,
        int value_index,
        _Bool use_locking,
        _Bool validate_shape,
        TF_CopyTensorFunc copy_func,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void assign_update_variable(
        int input_index,
        int value_index,
        int op,
        int is_variant_type,
        TF_CopyTensorFunc copy_func,
        TF_UpdateTensorFunc update_func,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void temporary_variable(
        TFDataTypeEnum dtype,
        const int64_t* dims,
        int num_dims,
        const ice::sonic::String& var_name,
        TF_PluginAllocatorFunc plugin_allocator,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void destroy_temporary_variable(
        int index,
        const ice::sonic::String& var_name,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void maybe_lock_variable_input_mutexes_in_order(
        _Bool do_lock,
        _Bool sparse,
        const int* inputs,
        size_t len,
        TF_CopyTensorFunc copy_func,
        TF_VariableInputLockHolder** out_lock_holder,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    release_variable_input_lock_holder(TF_VariableInputLockHolder* lock_holder) noexcept = 0;
    virtual void get_input_tensor_from_variable(
        int input,
        _Bool lock_held,
        _Bool is_variant_type,
        _Bool sparse,
        TF_CopyTensorFunc copy_func,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    forward_ref_input_to_ref_output(int32_t input_index, int32_t output_index) noexcept = 0;
    virtual void
    is_ref_input(int i, _Bool* out_is_ref, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_input_by_name(
        const ice::sonic::String& input_name,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void add_n_variant(
        TF_BinaryAddFunc binary_add_func,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void zeros_like_variant(
        TF_ZerosLikeFunc zeros_like_func,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    get_stream(TF_Stream** out_stream, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void run_async_done_callback(void* done_callback) noexcept = 0;
    virtual void get_random_generator(
        const ice::sonic::TF_RandomGeneratorOps& out_generator,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_OpKernelContext*)) noexcept
    {
        m_vtable = ::TF_OpKernelContextOps{
            .struct_size = TF_OFFSET_OF_END(::TF_OpKernelContextOps, get_random_generator),

            .create = create,
            .destroy =
                [](TF_OpKernelContext* handle) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(handle);
                self.destroy();
            },
            .num_inputs =
                [](TF_OpKernelContext* ctx, int* out_num) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.num_inputs(out_num);
            },
            .num_outputs =
                [](TF_OpKernelContext* ctx, int* out_num) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.num_outputs(out_num);
            },
            .get_input =
                [](TF_OpKernelContext* ctx,
                   int i,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_input(
                    i,
                    out_tensor,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .input_range =
                [](TF_OpKernelContext* ctx,
                   const TF_String* name,
                   TF_InputRange_Args* out_args) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.input_range(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    out_args
                );
            },
            .input_datatype =
                [](TF_OpKernelContext* ctx, int index, TFDataTypeEnum* out_type) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.input_datatype(index, out_type);
            },
            .set_output =
                [](TF_OpKernelContext* ctx,
                   int i,
                   const TF_Tensor* tensor,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.set_output(
                    i,
                    self.wrap(std::type_identity<ice::sonic::TF_TensorOps>{}, tensor),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_mutable_output =
                [](TF_OpKernelContext* ctx,
                   int i,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_mutable_output(
                    i,
                    out_tensor,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_serialized_function_def_library =
                [](TF_OpKernelContext* ctx,
                   TF_Buffer* serialized_function_def_library,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_serialized_function_def_library(
                    self.wrap(
                        std::type_identity<ice::sonic::TF_BufferOps>{},
                        serialized_function_def_library
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_serialized_config_proto =
                [](TF_OpKernelContext* ctx,
                   TF_Buffer* serialized_config_proto,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_serialized_config_proto(
                    self.wrap(
                        std::type_identity<ice::sonic::TF_BufferOps>{},
                        serialized_config_proto
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_serialized_resource_handle_proto =
                [](TF_OpKernelContext* ctx,
                   int i,
                   TF_Buffer* serialized_resource_handle_proto,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_serialized_resource_handle_proto(
                    i,
                    self.wrap(
                        std::type_identity<ice::sonic::TF_BufferOps>{},
                        serialized_resource_handle_proto
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .failure =
                [](TF_OpKernelContext* ctx, TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.failure(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .expected_output_datatype =
                [](TF_OpKernelContext* ctx, int i, TFDataTypeEnum* out_type) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.expected_output_datatype(i, out_type);
            },
            .is_host_memory_input =
                [](TF_OpKernelContext* ctx,
                   int i,
                   _Bool* out_is_host,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.is_host_memory_input(
                    i,
                    out_is_host,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .is_host_memory_output =
                [](TF_OpKernelContext* ctx,
                   int i,
                   _Bool* out_is_host,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.is_host_memory_output(
                    i,
                    out_is_host,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .step_id =
                [](TF_OpKernelContext* ctx, int64_t* out_id) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.step_id(out_id);
            },
            .get_frame_id =
                [](TF_OpKernelContext* ctx, uint64_t* out_id) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_frame_id(out_id);
            },
            .get_iter_id =
                [](TF_OpKernelContext* ctx, int64_t* out_id) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_iter_id(out_id);
            },
            .get_step_id =
                [](TF_OpKernelContext* ctx, int64_t* out_id) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_step_id(out_id);
            },
            .get_device_id =
                [](TF_OpKernelContext* ctx, int* out_id) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_device_id(out_id);
            },
            .get_device_name =
                [](TF_OpKernelContext* ctx, TF_String* out_name) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_device_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .get_graph_def_version =
                [](TF_OpKernelContext* ctx, int* out_version) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_graph_def_version(out_version);
            },
            .get_op_kernel_name =
                [](TF_OpKernelContext* ctx, TF_String* out_name) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_op_kernel_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name)
                );
            },
            .get_resource_mgr_default_container_name =
                [](TF_OpKernelContext* ctx, TF_String* out_name) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_resource_mgr_default_container_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name)
                );
            },
            .get_op_kernel_requested_input =
                [](TF_OpKernelContext* ctx, size_t index, TF_String* out_name) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_op_kernel_requested_input(
                    index,
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_name)
                );
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
                auto& self = TF_OpKernelContextOps::from_handle(context);
                self.allocate_output(
                    index,
                    dtype,
                    dims,
                    num_dims,
                    len,
                    out_tensor,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
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
                auto& self = TF_OpKernelContextOps::from_handle(context);
                self.forward_input_or_allocate_output(
                    candidate_input_indices,
                    num_candidate_input_indices,
                    output_index,
                    output_dims,
                    output_num_dims,
                    out_forwarded_input,
                    out_tensor,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
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
                auto& self = TF_OpKernelContextOps::from_handle(context);
                self.allocate_temp(
                    dtype,
                    dims,
                    num_dims,
                    alloc_attrs,
                    out_tensor,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .inc_num_deferred_ops =
                [](TF_OpKernelContext* context) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(context);
                self.inc_num_deferred_ops();
            },
            .dec_num_deferred_ops =
                [](TF_OpKernelContext* context) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(context);
                self.dec_num_deferred_ops();
            },
            .assign_variable =
                [](TF_OpKernelContext* ctx,
                   int input_index,
                   int value_index,
                   _Bool validate_shape,
                   TF_CopyTensorFunc copy_func,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.assign_variable(
                    input_index,
                    value_index,
                    validate_shape,
                    copy_func,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
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
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.assign_ref_variable(
                    input_ref_index,
                    output_ref_index,
                    value_index,
                    use_locking,
                    validate_shape,
                    copy_func,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
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
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.assign_update_variable(
                    input_index,
                    value_index,
                    op,
                    is_variant_type,
                    copy_func,
                    update_func,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
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
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.temporary_variable(
                    dtype,
                    dims,
                    num_dims,
                    self.wrap(std::type_identity<ice::sonic::String>{}, var_name),
                    plugin_allocator,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .destroy_temporary_variable =
                [](TF_OpKernelContext* ctx,
                   int index,
                   const TF_String* var_name,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.destroy_temporary_variable(
                    index,
                    self.wrap(std::type_identity<ice::sonic::String>{}, var_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
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
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.maybe_lock_variable_input_mutexes_in_order(
                    do_lock,
                    sparse,
                    inputs,
                    len,
                    copy_func,
                    out_lock_holder,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .release_variable_input_lock_holder =
                [](TF_OpKernelContext* ctx, TF_VariableInputLockHolder* lock_holder) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.release_variable_input_lock_holder(lock_holder);
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
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_input_tensor_from_variable(
                    input,
                    lock_held,
                    is_variant_type,
                    sparse,
                    copy_func,
                    out_tensor,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .forward_ref_input_to_ref_output =
                [](TF_OpKernelContext* ctx, int32_t input_index, int32_t output_index) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.forward_ref_input_to_ref_output(input_index, output_index);
            },
            .is_ref_input =
                [](TF_OpKernelContext* ctx,
                   int i,
                   _Bool* out_is_ref,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.is_ref_input(
                    i,
                    out_is_ref,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_input_by_name =
                [](TF_OpKernelContext* ctx,
                   const TF_String* input_name,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_input_by_name(
                    self.wrap(std::type_identity<ice::sonic::String>{}, input_name),
                    out_tensor,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .add_n_variant =
                [](TF_OpKernelContext* ctx,
                   TF_BinaryAddFunc binary_add_func,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.add_n_variant(
                    binary_add_func,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .zeros_like_variant =
                [](TF_OpKernelContext* ctx,
                   TF_ZerosLikeFunc zeros_like_func,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.zeros_like_variant(
                    zeros_like_func,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stream =
                [](TF_OpKernelContext* ctx, TF_Stream** out_stream, TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_stream(
                    out_stream,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .run_async_done_callback =
                [](TF_OpKernelContext* ctx, void* done_callback) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.run_async_done_callback(done_callback);
            },
            .get_random_generator =
                [](TF_OpKernelContext* ctx,
                   TF_RandomGenerator* out_generator,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelContextOps::from_handle(ctx);
                self.get_random_generator(
                    self.wrap(
                        std::type_identity<ice::sonic::TF_RandomGeneratorOps>{},
                        out_generator
                    ),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_BufferOps
    wrap(std::type_identity<ice::sonic::TF_BufferOps>, const ::TF_Buffer* handle) const noexcept
    {
        return ice::sonic::TF_BufferOps{m_TF_BufferOps_ops, const_cast<::TF_Buffer*>(handle)};
    }

    ice::sonic::TF_RandomGeneratorOps wrap(
        std::type_identity<ice::sonic::TF_RandomGeneratorOps>,
        const ::TF_RandomGenerator* handle
    ) const noexcept
    {
        return ice::sonic::TF_RandomGeneratorOps{
            m_TF_RandomGeneratorOps_ops,
            const_cast<::TF_RandomGenerator*>(handle)
        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    ice::sonic::TF_TensorOps
    wrap(std::type_identity<ice::sonic::TF_TensorOps>, const ::TF_Tensor* handle) const noexcept
    {
        return ice::sonic::TF_TensorOps{m_TF_TensorOps_ops, const_cast<::TF_Tensor*>(handle)};
    }

    const ::TF_OpKernelContextOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_OpKernelContext& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_OpKernelContextOps*>(&m_vtable));
    }

private:
    ::TF_OpKernelContextOps m_vtable;
    ::TF_OpKernelContext m_handle;

    const ::TF_BufferOps* m_TF_BufferOps_ops{nullptr};

    const ::TF_RandomGeneratorOps* m_TF_RandomGeneratorOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_TensorOps* m_TF_TensorOps_ops{nullptr};
};

} // namespace ice::builder
