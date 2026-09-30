// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/optimizer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/optimizer.h"

export module cc_ice_extern_grappler_builder:optimizer;

import std;

export namespace ice::builder {

class TFGrapplerOptimizerOps
{
public:
    TFGrapplerOptimizerOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFGrapplerOptimizerOps(const TFGrapplerOptimizerOps&) = delete;
    TFGrapplerOptimizerOps& operator=(const TFGrapplerOptimizerOps&) = delete;

    static TFGrapplerOptimizerOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGrapplerOptimizerOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerOptimizerOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGrapplerOptimizerOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerOptimizerOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> optimize(
        const ice::sonic::TF_BufferOps& graph_buf,
        const ice::sonic::TFGrapplerItemOps& item,
        const ice::sonic::TF_BufferOps& out_optimized_graph_buf
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGrapplerOptimizerOps{
            .struct_size = TF_RAPPLEROPTIMIZER_STRUCT_SIZE,
            .optimize = [](TFGrapplerOptimizer* optimizer,
                           const TF_Buffer* graph_buf,
                           const TFGrapplerItem* item,
                           TF_Buffer* out_optimized_graph_buf,
                           TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerOptimizerOps::from_handle(optimizer).optimize(
                    ice::sonic::TF_BufferOps::wrap(graph_buf),
                    ice::sonic::TFGrapplerItemOps::wrap(item),
                    ice::sonic::TF_BufferOps::wrap(out_optimized_graph_buf)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFGrapplerOptimizerOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFGrapplerOptimizer& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGrapplerOptimizerOps m_vtable;
    TFGrapplerOptimizer m_handle;
};

} // namespace ice::builder
