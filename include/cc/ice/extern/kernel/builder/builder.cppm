// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/builder.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/builder.h"

export module cc_ice_extern_kernel_builder:builder;

import std;

export namespace ice::builder {

class TF_KernelBuilderOps
{
public:
    TF_KernelBuilderOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::Status>
    type_constraint(const ice::sonic::String& attr_name, TFDataTypeEnum type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    host_memory(const ice::sonic::String& arg_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    priority(int32_t priority_number) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    label(const ice::sonic::String& label) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    register_kernel_builder(const ice::sonic::String& kernel_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> register_kernel_builder_with_kernel_def(
        const ice::sonic::String& serialized_kernel_def,
        const ice::sonic::String& name
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_KernelBuilderOps{
            .struct_size = TF_KERNELBUILDER_STRUCT_SIZE,
            .type_constraint =
                [](TF_KernelBuilder* kernel_builder,
                   const TF_String* attr_name,
                   TFDataTypeEnum type,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_KernelBuilderOps::from_handle(kernel_builder)
                               .type_constraint(ice::sonic::String::wrap(attr_name), type);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .host_memory =
                [](TF_KernelBuilder* kernel_builder, const TF_String* arg_name) noexcept
            {
                auto res = TF_KernelBuilderOps::from_handle(kernel_builder)
                               .host_memory(ice::sonic::String::wrap(arg_name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .priority =
                [](TF_KernelBuilder* kernel_builder, int32_t priority_number) noexcept
            {
                auto res =
                    TF_KernelBuilderOps::from_handle(kernel_builder).priority(priority_number);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .label =
                [](TF_KernelBuilder* kernel_builder, const TF_String* label) noexcept
            {
                auto res = TF_KernelBuilderOps::from_handle(kernel_builder)
                               .label(ice::sonic::String::wrap(label));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .register_kernel_builder =
                [](TF_KernelBuilder* builder,
                   const TF_String* kernel_name,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_KernelBuilderOps::from_handle(builder).register_kernel_builder(
                    ice::sonic::String::wrap(kernel_name)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .register_kernel_builder_with_kernel_def =
                [](TF_KernelBuilder* builder,
                   const TF_String* serialized_kernel_def,
                   const TF_String* name,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_KernelBuilderOps::from_handle(builder)
                               .register_kernel_builder_with_kernel_def(
                                   ice::sonic::String::wrap(serialized_kernel_def),
                                   ice::sonic::String::wrap(name)
                               );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_KernelBuilderOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_KernelBuilder& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_KernelBuilderOps m_vtable;
    TF_KernelBuilder m_handle;
};

} // namespace ice::builder
