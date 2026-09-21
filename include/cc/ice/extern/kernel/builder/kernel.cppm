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
    static TF_KernelOps* create(void* ctx) noexcept
    {
        return static_cast<TF_KernelOps*>(ctx);
    }

    template<typename HandleT>
    static TF_KernelOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_KernelOps*>(handle->plugin_data);
    }

    virtual ~TF_KernelOps() = default;

    static TF_KernelOps* get_generic_vtable()
    {
        static TF_KernelOps vtable = {
            .struct_size = TF_KERNEL_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_KernelOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_KernelOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
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
