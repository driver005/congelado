// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/builder.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/builder.h"

export module cc_ice_extern_kernel_sonic:builder;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_KernelBuilderOps : public ice::sonic::Runtime<TF_KernelBuilderOps, TF_KernelBuilderOps>
{
public:
    explicit TF_KernelBuilderOps(TF_KernelBuilderOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "kernel";

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    type_constraint(const ice::sonic::String& attr_name, TFDataTypeEnum type) noexcept
    {
        ice::sonic::Status status;
        m_ops->type_constraint(get_handle(), attr_name.get_handle(), type, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void host_memory(const ice::sonic::String& arg_name) noexcept
    {
        m_ops->host_memory(get_handle(), arg_name.get_handle());
    }

    void priority(int32_t priority_number) noexcept
    {
        m_ops->priority(get_handle(), priority_number);
    }

    void label(const ice::sonic::String& label) noexcept
    {
        m_ops->label(get_handle(), label.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    register_kernel_builder(const ice::sonic::String& kernel_name) noexcept
    {
        ice::sonic::Status status;
        m_ops->register_kernel_builder(get_handle(), kernel_name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> register_kernel_builder_with_kernel_def(
        const ice::sonic::String& serialized_kernel_def,
        const ice::sonic::String& name
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->register_kernel_builder_with_kernel_def(
            get_handle(),
            serialized_kernel_def.get_handle(),
            name.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
