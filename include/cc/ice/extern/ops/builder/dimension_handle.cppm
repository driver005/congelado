// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/dimension_handle.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/dimension_handle.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_ops_builder:dimension_handle;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class DimensionHandle
{
public:
    explicit DimensionHandle() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    DimensionHandle(const DimensionHandle&) = delete;
    DimensionHandle& operator=(const DimensionHandle&) = delete;

    static DimensionHandle& from_handle(void* ctx) noexcept
    {
        return *static_cast<DimensionHandle*>(ctx);
    }

    template<typename HandleT>
    static DimensionHandle& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<DimensionHandle*>(handle->plugin_data);
    }

    virtual ~DimensionHandle() = default;
    virtual void destroy() noexcept = 0;
    virtual void value_known(int& out_known) noexcept = 0;
    virtual void value(std::int64_t& out_value) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_DimensionHandle*)) noexcept
    {
        m_vtable = ::TF_DimensionHandleOps{
            .struct_size = TF_OFFSET_OF_END(::TF_DimensionHandleOps, value),

            .create = create,
            .destroy =
                [](::TF_DimensionHandle* handle) noexcept
            {
                auto& self = DimensionHandle::from_handle(handle);
                self.destroy();
            },
            .value_known =
                [](::TF_DimensionHandle* dim_handle, int* out_known) noexcept
            {
                auto& self = DimensionHandle::from_handle(dim_handle);
                self.value_known(*out_known);
            },
            .value =
                [](::TF_DimensionHandle* dim_handle, int64_t* out_value) noexcept
            {
                auto& self = DimensionHandle::from_handle(dim_handle);
                self.value(*out_value);
            },

        };
    }

    const ::TF_DimensionHandleOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_DimensionHandle& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_DimensionHandleOps*>(&m_vtable));
    }

private:
    ::TF_DimensionHandleOps m_vtable;
    ::TF_DimensionHandle m_handle;
};

} // namespace ice::builder
