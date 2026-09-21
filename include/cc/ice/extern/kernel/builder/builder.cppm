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
    static TF_KernelBuilderOps* create(void* ctx) noexcept
    {
        return static_cast<TF_KernelBuilderOps*>(ctx);
    }

    template<typename HandleT>
    static TF_KernelBuilderOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_KernelBuilderOps*>(handle->plugin_data);
    }

    virtual ~TF_KernelBuilderOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    type_constraint(const char* attr_name, TFDataTypeEnum type) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    host_memory(const char* arg_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    priority(int32_t priority_number) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> label(const char* label) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    register_kernel_builder(const char* kernel_name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> register_kernel_builder_with_kernel_def(
        const char* serialized_kernel_def,
        const char* name
    ) noexcept = 0;

    static TF_KernelBuilderOps* get_generic_vtable()
    {
        static TF_KernelBuilderOps vtable = {
            .struct_size = TF_KERNELBUILDER_STRUCT_SIZE,
            .type_constraint =
                [](TF_KernelBuilder* kernel_builder,
                   const char* attr_name,
                   TFDataTypeEnum type,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_KernelBuilderOps::create(kernel_builder);
                auto res = self->type_constraint(attr_name, type);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .host_memory =
                [](TF_KernelBuilder* kernel_builder, const char* arg_name) noexcept
            {
                auto* self = TF_KernelBuilderOps::create(kernel_builder);
                auto res = self->host_memory(arg_name);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .priority =
                [](TF_KernelBuilder* kernel_builder, int32_t priority_number) noexcept
            {
                auto* self = TF_KernelBuilderOps::create(kernel_builder);
                auto res = self->priority(priority_number);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .label =
                [](TF_KernelBuilder* kernel_builder, const char* label) noexcept
            {
                auto* self = TF_KernelBuilderOps::create(kernel_builder);
                auto res = self->label(label);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .register_kernel_builder =
                [](TF_KernelBuilder* builder,
                   const char* kernel_name,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_KernelBuilderOps::create(builder);
                auto res = self->register_kernel_builder(kernel_name);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .register_kernel_builder_with_kernel_def =
                [](TF_KernelBuilder* builder,
                   const char* serialized_kernel_def,
                   const char* name,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_KernelBuilderOps::create(builder);
                auto res =
                    self->register_kernel_builder_with_kernel_def(serialized_kernel_def, name);
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
