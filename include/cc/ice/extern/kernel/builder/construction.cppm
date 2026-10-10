// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/construction.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/construction.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_kernel_builder:construction;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_OpKernelConstructionOps
{
public:
    explicit TF_OpKernelConstructionOps(
        const ::TF_BufferOps* TF_BufferOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_BufferOps_ops = TF_BufferOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void failure(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_node_def(
        const ice::sonic::TF_BufferOps& buffer,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_size(
        const ice::sonic::String& attr_name,
        int32_t* out_list_size,
        int32_t* out_total_size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_type(
        const ice::sonic::String& attr_name,
        TFDataTypeEnum* out_val,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_int32(
        const ice::sonic::String& attr_name,
        int32_t* out_val,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_int64(
        const ice::sonic::String& attr_name,
        int64_t* out_val,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_float(
        const ice::sonic::String& attr_name,
        float* out_val,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_bool(
        const ice::sonic::String& attr_name,
        _Bool* out_val,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_string(
        const ice::sonic::String& attr_name,
        const ice::sonic::String& out_val,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_tensor(
        const ice::sonic::String& attr_name,
        TF_Tensor** out_val,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_type_list(
        const ice::sonic::String& attr_name,
        TFDataTypeEnum* out_vals,
        int max_vals,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_int32_list(
        const ice::sonic::String& attr_name,
        int32_t* out_vals,
        int max_vals,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_int64_list(
        const ice::sonic::String& attr_name,
        int64_t* out_vals,
        int max_vals,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_float_list(
        const ice::sonic::String& attr_name,
        float* out_vals,
        int max_vals,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_bool_list(
        const ice::sonic::String& attr_name,
        _Bool* out_vals,
        int max_vals,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_string_list(
        const ice::sonic::String& attr_name,
        char** out_values,
        size_t* out_lengths,
        int max_values,
        void* storage,
        size_t storage_size,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_tensor_list(
        const ice::sonic::String& attr_name,
        TF_Tensor** out_vals,
        int max_values,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_attr_function(
        const ice::sonic::String& attr_name,
        const ice::sonic::TF_BufferOps& buffer,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void has_attr(
        const ice::sonic::String& attr_name,
        _Bool* out_has_attr,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void get_attr_tensor_shape(
        const ice::sonic::String& attr_name,
        int64_t* out_dims,
        size_t num_dims,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_OpKernelConstruction*)) noexcept
    {
        m_vtable = ::TF_OpKernelConstructionOps{
            .struct_size = TF_OFFSET_OF_END(::TF_OpKernelConstructionOps, get_attr_tensor_shape),

            .create = create,
            .destroy =
                [](TF_OpKernelConstruction* handle) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(handle);
                self.destroy();
            },
            .failure =
                [](TF_OpKernelConstruction* ctx, TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.failure(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .get_node_def =
                [](TF_OpKernelConstruction* ctx, TF_Buffer* buffer, TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_node_def(
                    self.wrap(std::type_identity<ice::sonic::TF_BufferOps>{}, buffer),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_size =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int32_t* out_list_size,
                   int32_t* out_total_size,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_size(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_list_size,
                    out_total_size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_type =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   TFDataTypeEnum* out_val,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_type(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_val,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_int32 =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int32_t* out_val,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_int32(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_val,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_int64 =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int64_t* out_val,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_int64(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_val,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_float =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   float* out_val,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_float(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_val,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_bool =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   _Bool* out_val,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_bool(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_val,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_string =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   TF_String* out_val,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_string(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_val),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_tensor =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   TF_Tensor** out_val,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_tensor(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_val,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_type_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   TFDataTypeEnum* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_type_list(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_vals,
                    max_vals,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_int32_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int32_t* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_int32_list(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_vals,
                    max_vals,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_int64_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int64_t* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_int64_list(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_vals,
                    max_vals,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_float_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   float* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_float_list(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_vals,
                    max_vals,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_bool_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   _Bool* out_vals,
                   int max_vals,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_bool_list(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_vals,
                    max_vals,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
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
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_string_list(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_values,
                    out_lengths,
                    max_values,
                    storage,
                    storage_size,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_tensor_list =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   TF_Tensor** out_vals,
                   int max_values,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_tensor_list(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_vals,
                    max_values,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_attr_function =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   TF_Buffer* buffer,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_function(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    self.wrap(std::type_identity<ice::sonic::TF_BufferOps>{}, buffer),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .has_attr =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   _Bool* out_has_attr,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.has_attr(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_has_attr,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_name =
                [](TF_OpKernelConstruction* ctx, TF_String* out_name) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .get_attr_tensor_shape =
                [](TF_OpKernelConstruction* ctx,
                   const TF_String* attr_name,
                   int64_t* out_dims,
                   size_t num_dims,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_OpKernelConstructionOps::from_handle(ctx);
                self.get_attr_tensor_shape(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    out_dims,
                    num_dims,
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

    const ::TF_OpKernelConstructionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_OpKernelConstruction& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_OpKernelConstructionOps*>(&m_vtable)
        );
    }

private:
    ::TF_OpKernelConstructionOps m_vtable;
    ::TF_OpKernelConstruction m_handle;

    const ::TF_BufferOps* m_TF_BufferOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
