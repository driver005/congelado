// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/construction.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/construction.h"

export module cc_ice_extern_kernel_builder:construction;

import std;

export namespace ice::builder {

class TF_OpKernelConstructionOps
{
public:
    static TF_OpKernelConstructionOps* create(void* ctx) noexcept
    {
        return static_cast<TF_OpKernelConstructionOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OpKernelConstructionOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_OpKernelConstructionOps*>(handle->plugin_data);
    }

    virtual ~TF_OpKernelConstructionOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> failure() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_node_def(const ice::sonic::TF_BufferOps& buffer) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_attr_size(
        const char* attr_name,
        int32_t* out_list_size,
        int32_t* out_total_size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_type(const char* attr_name, TFDataTypeEnum* out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_int32(const char* attr_name, int32_t* out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_int64(const char* attr_name, int64_t* out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_float(const char* attr_name, float* out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_bool(const char* attr_name, _Bool* out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_string(const char* attr_name, char* out_val, size_t max_length) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_tensor(const char* attr_name, TF_Tensor** out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_type_list(const char* attr_name, TFDataTypeEnum* out_vals, int max_vals) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_int32_list(const char* attr_name, int32_t* out_vals, int max_vals) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_int64_list(const char* attr_name, int64_t* out_vals, int max_vals) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_float_list(const char* attr_name, float* out_vals, int max_vals) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_bool_list(const char* attr_name, _Bool* out_vals, int max_vals) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_attr_string_list(
        const char* attr_name,
        char** out_values,
        size_t* out_lengths,
        int max_values,
        void* storage,
        size_t storage_size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_tensor_list(const char* attr_name, TF_Tensor** out_vals, int max_values) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_function(const char* attr_name, const ice::sonic::TF_BufferOps& buffer) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    has_attr(const char* attr_name, _Bool* out_has_attr) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_attr_tensor_shape(const char* attr_name, int64_t* out_dims, size_t num_dims) noexcept = 0;

    static TF_OpKernelConstructionOps* get_generic_vtable()
    {
        static TF_OpKernelConstructionOps vtable = {
            .struct_size = TF_OPKERNELCONSTRUCTION_STRUCT_SIZE,
            .failure =
                [](TF_OpKernelConstruction* ctx, TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->failure();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_node_def =
                [](TF_OpKernelConstruction* ctx, TF_Buffer* buffer, TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_node_def(ice::sonic::TF_BufferOps::wrap(buffer));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_size =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   int32_t* out_list_size,
                   int32_t* out_total_size,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_size(attr_name, out_list_size, out_total_size);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_type =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   TFDataTypeEnum* out_val,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_type(attr_name, out_val);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_int32 =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   int32_t* out_val,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_int32(attr_name, out_val);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_int64 =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   int64_t* out_val,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_int64(attr_name, out_val);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_float =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   float* out_val,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_float(attr_name, out_val);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_bool =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   _Bool* out_val,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_bool(attr_name, out_val);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_string =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   char* out_val,
                   size_t max_length,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_string(attr_name, out_val, max_length);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_tensor =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   TF_Tensor** out_val,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_tensor(attr_name, out_val);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_type_list =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   TFDataTypeEnum* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_type_list(attr_name, out_vals, max_vals);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_int32_list =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   int32_t* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_int32_list(attr_name, out_vals, max_vals);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_int64_list =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   int64_t* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_int64_list(attr_name, out_vals, max_vals);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_float_list =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   float* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_float_list(attr_name, out_vals, max_vals);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_bool_list =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   _Bool* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_bool_list(attr_name, out_vals, max_vals);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_string_list =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   char** out_values,
                   size_t* out_lengths,
                   int max_values,
                   void* storage,
                   size_t storage_size,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_string_list(
                    attr_name,
                    out_values,
                    out_lengths,
                    max_values,
                    storage,
                    storage_size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_tensor_list =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   TF_Tensor** out_vals,
                   int max_values,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_tensor_list(attr_name, out_vals, max_values);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_function =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   TF_Buffer* buffer,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res =
                    self->get_attr_function(attr_name, ice::sonic::TF_BufferOps::wrap(buffer));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .has_attr =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   _Bool* out_has_attr,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->has_attr(attr_name, out_has_attr);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .get_attr_tensor_shape =
                [](TF_OpKernelConstruction* ctx,
                   const char* attr_name,
                   int64_t* out_dims,
                   size_t num_dims,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_OpKernelConstructionOps::create(ctx);
                auto res = self->get_attr_tensor_shape(attr_name, out_dims, num_dims);
                if (!res) {
                    res.error().to_c(out_status);
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
