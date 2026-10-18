// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/kernel/kernel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/kernel/kernel.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_kernel_builder:kernel;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Kernel
{
public:
    explicit Kernel(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    Kernel(const Kernel&) = delete;
    Kernel& operator=(const Kernel&) = delete;

    static Kernel& from_handle(void* ctx) noexcept
    {
        return *static_cast<Kernel*>(ctx);
    }

    template<typename HandleT>
    static Kernel& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Kernel*>(handle->plugin_data);
    }

    virtual ~Kernel() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Kernel*)) noexcept
    {
        m_vtable = ::TF_KernelOps{
            .struct_size = TF_OFFSET_OF_END(::TF_KernelOps, get_name),

            .create = create,
            .destroy =
                [](::TF_Kernel* handle) noexcept
            {
                auto& self = Kernel::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TF_Kernel* kernel, ::TF_String* out_name) noexcept
            {
                auto& self = Kernel::from_handle(kernel);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TF_KernelOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Kernel& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_KernelOps*>(&m_vtable));
    }

private:
    ::TF_KernelOps m_vtable;
    ::TF_Kernel m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
