// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/kernel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/kernel.h"

export module cc_ice_extern_kernel_builder:kernel;

import std;

export namespace ice::builder {

class TF_KernelOps
{
public:
    TF_KernelOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_KernelOps(const TF_KernelOps&) = delete;
    TF_KernelOps& operator=(const TF_KernelOps&) = delete;

    static TF_KernelOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_KernelOps*>(ctx);
    }

    template<typename HandleT>
    static TF_KernelOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_KernelOps*>(handle->plugin_data);
    }

    virtual ~TF_KernelOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_KernelOps{
            .struct_size = TF_KERNEL_STRUCT_SIZE,
            .destroy =
                [](TF_Kernel* kernel) noexcept
            {
                TF_KernelOps::from_handle(kernel).destroy();
            },
            .get_name =
                [](TF_Kernel* kernel, TF_String* out_name) noexcept
            {
                TF_KernelOps::from_handle(kernel).get_name(ice::sonic::String::wrap(out_name));
            },

        };
    }

    const ::TF_KernelOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Kernel& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_KernelOps m_vtable;
    TF_Kernel m_handle;
};

} // namespace ice::builder
