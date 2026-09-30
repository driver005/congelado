// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/dimension_handle.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/dimension_handle.h"

export module cc_ice_extern_ops_builder:dimension_handle;

import std;

export namespace ice::builder {

class TF_DimensionHandleOps
{
public:
    TF_DimensionHandleOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_DimensionHandleOps(const TF_DimensionHandleOps&) = delete;
    TF_DimensionHandleOps& operator=(const TF_DimensionHandleOps&) = delete;

    static TF_DimensionHandleOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_DimensionHandleOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DimensionHandleOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_DimensionHandleOps*>(handle->plugin_data);
    }

    virtual ~TF_DimensionHandleOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> value_known(int* out_known) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> value(int64_t* out_value) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_DimensionHandleOps{
            .struct_size = TF_DIMENSIONHANDLE_STRUCT_SIZE,
            .value_known =
                [](TF_DimensionHandle* dim_handle, int* out_known) noexcept
            {
                auto res = TF_DimensionHandleOps::from_handle(dim_handle).value_known(out_known);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .value =
                [](TF_DimensionHandle* dim_handle, int64_t* out_value) noexcept
            {
                auto res = TF_DimensionHandleOps::from_handle(dim_handle).value(out_value);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_DimensionHandleOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_DimensionHandle& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_DimensionHandleOps m_vtable;
    TF_DimensionHandle m_handle;
};

} // namespace ice::builder
