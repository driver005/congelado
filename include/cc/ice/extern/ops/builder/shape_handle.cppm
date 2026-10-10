// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/shape_handle.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/shape_handle.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_ops_builder:shape_handle;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ShapeHandleOps
{
public:
    explicit TF_ShapeHandleOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_ShapeHandleOps(const TF_ShapeHandleOps&) = delete;
    TF_ShapeHandleOps& operator=(const TF_ShapeHandleOps&) = delete;

    static TF_ShapeHandleOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ShapeHandleOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ShapeHandleOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ShapeHandleOps*>(handle->plugin_data);
    }

    virtual ~TF_ShapeHandleOps() = default;
    virtual void destroy() noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_ShapeHandle*)) noexcept
    {
        m_vtable = ::TF_ShapeHandleOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ShapeHandleOps, destroy),

            .create = create,
            .destroy = [](TF_ShapeHandle* handle) noexcept
            {
                auto& self = TF_ShapeHandleOps::from_handle(handle);
                self.destroy();
            },

        };
    }

    const ::TF_ShapeHandleOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_ShapeHandle& get_handle() const noexcept
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
            const_cast<::TF_ShapeHandleOps*>(&m_vtable)
        );
    }

private:
    ::TF_ShapeHandleOps m_vtable;
    ::TF_ShapeHandle m_handle;
};

} // namespace ice::builder
