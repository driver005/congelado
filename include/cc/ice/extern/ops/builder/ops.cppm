// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/ops/ops.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/ops/ops.h"

export module cc_ice_extern_ops_builder:ops;

import std;

export namespace ice::builder {

class TF_OpsOps
{
public:
    static TF_OpsOps* create(void* ctx) noexcept
    {
        return static_cast<TF_OpsOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OpsOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_OpsOps*>(handle->plugin_data);
    }

    virtual ~TF_OpsOps() = default;

    static TF_OpsOps* get_generic_vtable()
    {
        static TF_OpsOps vtable = {
            .struct_size = TF_OPS_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_OpsOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_OpsOps::create(plugin_context);
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
