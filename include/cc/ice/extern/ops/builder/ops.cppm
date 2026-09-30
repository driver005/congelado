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
    TF_OpsOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_OpsOps(const TF_OpsOps&) = delete;
    TF_OpsOps& operator=(const TF_OpsOps&) = delete;

    static TF_OpsOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_OpsOps*>(ctx);
    }

    template<typename HandleT>
    static TF_OpsOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_OpsOps*>(handle->plugin_data);
    }

    virtual ~TF_OpsOps() = default;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_OpsOps{
            .struct_size = TF_OPS_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_OpsOps>{&TF_OpsOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_OpsOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },

        };
    }

    const ::TF_OpsOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Ops& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_OpsOps m_vtable;
    TF_Ops m_handle;
};

} // namespace ice::builder
