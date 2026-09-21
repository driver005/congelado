// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/dimension_handle.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/dimension_handle.h"

export module cc_ice_builder_ops:dimension_handle;

import std;

export namespace ice::builder {

class TF_DimensionHandleOps
{
public:
    static TF_DimensionHandleOps* create(void* ctx) noexcept
    {
        return static_cast<TF_DimensionHandleOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DimensionHandleOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_DimensionHandleOps*>(handle->plugin_data);
    }

    virtual ~TF_DimensionHandleOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> value_known(int* out_known) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> value(int64_t* out_value) noexcept = 0;

    static TF_DimensionHandleOps* get_generic_vtable()
    {
        static TF_DimensionHandleOps vtable = {
            .struct_size = TF_DIMENSIONHANDLE_STRUCT_SIZE,
            .value_known =
                [](TF_DimensionHandle* dim_handle, int* out_known) noexcept
            {
                auto* self = TF_DimensionHandleOps::create(dim_handle);
                auto res = self->value_known(out_known);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .value =
                [](TF_DimensionHandle* dim_handle, int64_t* out_value) noexcept
            {
                auto* self = TF_DimensionHandleOps::create(dim_handle);
                auto res = self->value(out_value);
                if (!res) {
                    res.error().to_c(status);
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
