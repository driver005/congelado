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
    static TFGrapplerOptimizerOps* create(void* ctx) noexcept
    {
        return static_cast<TFGrapplerOptimizerOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerOptimizerOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGrapplerOptimizerOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerOptimizerOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> optimize(
        const ice::sonic::TF_BufferOps& graph_buf,
        const ice::sonic::TFGrapplerItemOps& item,
        const ice::sonic::TF_BufferOps& out_optimized_graph_buf
    ) noexcept = 0;

    static TFGrapplerOptimizerOps* get_generic_vtable()
    {
        static TFGrapplerOptimizerOps vtable = {
            .struct_size = TF_RAPPLEROPTIMIZER_STRUCT_SIZE,
            .optimize = [](TFGrapplerOptimizer* optimizer,
                           const TF_Buffer* graph_buf,
                           const TFGrapplerItem* item,
                           TF_Buffer* out_optimized_graph_buf,
                           TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerOptimizerOps::create(optimizer);
                auto res = self->optimize(
                    ice::sonic::TF_BufferOps::wrap(graph_buf),
                    ice::sonic::TFGrapplerItemOps::wrap(item),
                    ice::sonic::TF_BufferOps::wrap(out_optimized_graph_buf)
                );
                if (!res) {
                    res.error().to_c(out_status);
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
