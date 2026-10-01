// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/builder.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/builder.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_kernel_builder:builder;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_KernelBuilderOps
{
public:
    explicit TF_KernelBuilderOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TF_KernelBuilderOps(const TF_KernelBuilderOps&) = delete;
    TF_KernelBuilderOps& operator=(const TF_KernelBuilderOps&) = delete;

    static TF_KernelBuilderOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_KernelBuilderOps*>(ctx);
    }

    template<typename HandleT>
    static TF_KernelBuilderOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_KernelBuilderOps*>(handle->plugin_data);
    }

    virtual ~TF_KernelBuilderOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void type_constraint(
        const ice::sonic::String& attr_name,
        TFDataTypeEnum type,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void host_memory(const ice::sonic::String& arg_name) noexcept = 0;
    virtual void priority(int32_t priority_number) noexcept = 0;
    virtual void label(const ice::sonic::String& label) noexcept = 0;
    virtual void register_kernel_builder(
        const ice::sonic::String& kernel_name,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void register_kernel_builder_with_kernel_def(
        const ice::sonic::String& serialized_kernel_def,
        const ice::sonic::String& name,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_KernelBuilder*)) noexcept
    {
        m_vtable = ::TF_KernelBuilderOps{
            .struct_size =
                TF_OFFSET_OF_END(::TF_KernelBuilderOps, register_kernel_builder_with_kernel_def),

            .create = create,
            .destroy =
                [](TF_KernelBuilder* handle) noexcept
            {
                auto& self = TF_KernelBuilderOps::from_handle(handle);
                self.destroy();
            },
            .type_constraint =
                [](TF_KernelBuilder* kernel_builder,
                   const TF_String* attr_name,
                   TFDataTypeEnum type,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_KernelBuilderOps::from_handle(kernel_builder);
                self.type_constraint(
                    self.wrap(std::type_identity<ice::sonic::String>{}, attr_name),
                    type,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .host_memory =
                [](TF_KernelBuilder* kernel_builder, const TF_String* arg_name) noexcept
            {
                auto& self = TF_KernelBuilderOps::from_handle(kernel_builder);
                self.host_memory(self.wrap(std::type_identity<ice::sonic::String>{}, arg_name));
            },
            .priority =
                [](TF_KernelBuilder* kernel_builder, int32_t priority_number) noexcept
            {
                auto& self = TF_KernelBuilderOps::from_handle(kernel_builder);
                self.priority(priority_number);
            },
            .label =
                [](TF_KernelBuilder* kernel_builder, const TF_String* label) noexcept
            {
                auto& self = TF_KernelBuilderOps::from_handle(kernel_builder);
                self.label(self.wrap(std::type_identity<ice::sonic::String>{}, label));
            },
            .register_kernel_builder =
                [](TF_KernelBuilder* builder,
                   const TF_String* kernel_name,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_KernelBuilderOps::from_handle(builder);
                self.register_kernel_builder(
                    self.wrap(std::type_identity<ice::sonic::String>{}, kernel_name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .register_kernel_builder_with_kernel_def =
                [](TF_KernelBuilder* builder,
                   const TF_String* serialized_kernel_def,
                   const TF_String* name,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_KernelBuilderOps::from_handle(builder);
                self.register_kernel_builder_with_kernel_def(
                    self.wrap(std::type_identity<ice::sonic::String>{}, serialized_kernel_def),
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

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

    const ::TF_KernelBuilderOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_KernelBuilder& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_KernelBuilderOps*>(&m_vtable));
    }

private:
    ::TF_KernelBuilderOps m_vtable;
    ::TF_KernelBuilder m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
