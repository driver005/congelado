// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/builder.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/builder.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_kernel_sonic:builder;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_KernelBuilderOps : public ice::sonic::Runtime<::TF_KernelBuilderOps, ::TF_KernelBuilder>
{
public:
    TF_KernelBuilderOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_KernelBuilderOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_KernelBuilder* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_KernelBuilderOps(const ::TF_KernelBuilderOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_KernelBuilderOps(const ::TF_KernelBuilderOps* ops, ::TF_KernelBuilder* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void type_constraint(
        const ice::sonic::String& attr_name,
        TFDataTypeEnum type,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->type_constraint(get_handle(), attr_name.get_handle(), type, out_status.get_handle());
    }

    void host_memory(const ice::sonic::String& arg_name) const noexcept
    {
        m_ops->host_memory(get_handle(), arg_name.get_handle());
    }

    void priority(int32_t priority_number) const noexcept
    {
        m_ops->priority(get_handle(), priority_number);
    }

    void label(const ice::sonic::String& label) const noexcept
    {
        m_ops->label(get_handle(), label.get_handle());
    }

    void register_kernel_builder(
        const ice::sonic::String& kernel_name,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->register_kernel_builder(
            get_handle(),
            kernel_name.get_handle(),
            out_status.get_handle()
        );
    }

    void register_kernel_builder_with_kernel_def(
        const ice::sonic::String& serialized_kernel_def,
        const ice::sonic::String& name,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->register_kernel_builder_with_kernel_def(
            get_handle(),
            serialized_kernel_def.get_handle(),
            name.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
