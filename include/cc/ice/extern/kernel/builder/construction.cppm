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
    TF_OpKernelConstructionOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_OpKernelConstructionOps(const TF_OpKernelConstructionOps&) = delete;
    TF_OpKernelConstructionOps& operator=(const TF_OpKernelConstructionOps&) = delete;

    static TF_OpKernelConstructionOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_OpKernelConstructionOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OpKernelConstructionOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_OpKernelConstructionOps*>(handle->plugin_data);
    }

    virtual ~TF_OpKernelConstructionOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> failure() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_node_def(const ice::sonic::TF_BufferOps& buffer) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_attr_size(
        const ice::sonic::String& attr_name,
        int32_t* out_list_size,
        int32_t* out_total_size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_attr_type(const ice::sonic::String& attr_name, TFDataTypeEnum* out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_attr_int32(const ice::sonic::String& attr_name, int32_t* out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_attr_int64(const ice::sonic::String& attr_name, int64_t* out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_attr_float(const ice::sonic::String& attr_name, float* out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_attr_bool(const ice::sonic::String& attr_name, _Bool* out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_attr_string(
        const ice::sonic::String& attr_name,
        const ice::sonic::String& out_val
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_attr_tensor(const ice::sonic::String& attr_name, TF_Tensor** out_val) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_attr_type_list(
        const ice::sonic::String& attr_name,
        TFDataTypeEnum* out_vals,
        int max_vals
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_attr_int32_list(
        const ice::sonic::String& attr_name,
        int32_t* out_vals,
        int max_vals
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_attr_int64_list(
        const ice::sonic::String& attr_name,
        int64_t* out_vals,
        int max_vals
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_attr_float_list(
        const ice::sonic::String& attr_name,
        float* out_vals,
        int max_vals
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_attr_bool_list(
        const ice::sonic::String& attr_name,
        _Bool* out_vals,
        int max_vals
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_attr_string_list(
        const ice::sonic::String& attr_name,
        char** out_values,
        size_t* out_lengths,
        int max_values,
        void* storage,
        size_t storage_size
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_attr_tensor_list(
        const ice::sonic::String& attr_name,
        TF_Tensor** out_vals,
        int max_values
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_attr_function(
        const ice::sonic::String& attr_name,
        const ice::sonic::TF_BufferOps& buffer
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    has_attr(const ice::sonic::String& attr_name, _Bool* out_has_attr) noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_attr_tensor_shape(
        const ice::sonic::String& attr_name,
        int64_t* out_dims,
        size_t num_dims
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_OpKernelConstructionOps{
            .struct_size = TF_OPKERNELCONSTRUCTION_STRUCT_SIZE,
            .failure =
                [](TF_OpKernelConstruction* ctx, TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).failure();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_node_def =
                [](TF_OpKernelConstruction* ctx, TF_Buffer* buffer, TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_node_def(
                    ice::sonic::TF_BufferOps::wrap(buffer)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_size =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int32_t* out_list_size,
                   int32_t* out_total_size,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_size(
                    ice::sonic::String::wrap(attr_name),
                    out_list_size,
                    out_total_size
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_type =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   TFDataTypeEnum* out_val,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_type(
                    ice::sonic::String::wrap(attr_name),
                    out_val
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_int32 =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int32_t* out_val,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_int32(
                    ice::sonic::String::wrap(attr_name),
                    out_val
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_int64 =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int64_t* out_val,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_int64(
                    ice::sonic::String::wrap(attr_name),
                    out_val
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_float =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   float* out_val,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_float(
                    ice::sonic::String::wrap(attr_name),
                    out_val
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_bool =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   _Bool* out_val,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_bool(
                    ice::sonic::String::wrap(attr_name),
                    out_val
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_string =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   TF_String* out_val,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_string(
                    ice::sonic::String::wrap(attr_name),
                    ice::sonic::String::wrap(out_val)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_tensor =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   TF_Tensor** out_val,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_tensor(
                    ice::sonic::String::wrap(attr_name),
                    out_val
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_type_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   TFDataTypeEnum* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_type_list(
                    ice::sonic::String::wrap(attr_name),
                    out_vals,
                    max_vals
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_int32_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int32_t* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_int32_list(
                    ice::sonic::String::wrap(attr_name),
                    out_vals,
                    max_vals
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_int64_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int64_t* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_int64_list(
                    ice::sonic::String::wrap(attr_name),
                    out_vals,
                    max_vals
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_float_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   float* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_float_list(
                    ice::sonic::String::wrap(attr_name),
                    out_vals,
                    max_vals
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_bool_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   _Bool* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_bool_list(
                    ice::sonic::String::wrap(attr_name),
                    out_vals,
                    max_vals
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_string_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   char** out_values,
                   size_t* out_lengths,
                   int max_values,
                   void* storage,
                   size_t storage_size,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_string_list(
                    ice::sonic::String::wrap(attr_name),
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
                   const TF_String* attr_name,
                   TF_Tensor** out_vals,
                   int max_values,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_tensor_list(
                    ice::sonic::String::wrap(attr_name),
                    out_vals,
                    max_values
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_attr_function =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   TF_Buffer* buffer,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_function(
                    ice::sonic::String::wrap(attr_name),
                    ice::sonic::TF_BufferOps::wrap(buffer)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .has_attr =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   _Bool* out_has_attr,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).has_attr(
                    ice::sonic::String::wrap(attr_name),
                    out_has_attr
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_name =
                [](TF_OpKernelConstruction* ctx, TF_String* out_name) noexcept
            {
                TF_OpKernelConstructionOps::from_handle(ctx).get_name(
                    ice::sonic::String::wrap(out_name)
                );
            },
            .get_attr_tensor_shape =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int64_t* out_dims,
                   size_t num_dims,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_OpKernelConstructionOps::from_handle(ctx).get_attr_tensor_shape(
                    ice::sonic::String::wrap(attr_name),
                    out_dims,
                    num_dims
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_OpKernelConstructionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_OpKernelConstruction& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_OpKernelConstructionOps m_vtable;
    TF_OpKernelConstruction m_handle;
};

} // namespace ice::builder
