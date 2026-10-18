// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/optimizer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/optimizer.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_grappler_sonic:optimizer;

import std;
import :item;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class GrapplerOptimizer :
    public ice::sonic::Runtime<::TFGrapplerOptimizerOps, ::TFGrapplerOptimizer>
{
public:
    GrapplerOptimizer(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    GrapplerOptimizer(
        const ice::sonic::Registration& registry,
        const ::TFGrapplerOptimizer& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit GrapplerOptimizer(const ::TFGrapplerOptimizerOps* ops) noexcept :
        Runtime(ops)
    {
    }

    GrapplerOptimizer(
        const ::TFGrapplerOptimizerOps* ops,
        const ::TFGrapplerOptimizer* handle
    ) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void optimize(
        const ice::sonic::Buffer& graph_buf,
        const ice::sonic::GrapplerItem& item,
        const ice::sonic::Buffer& out_optimized_graph_buf,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->optimize(
            get_handle(),
            graph_buf.get_handle(),
            item.get_handle(),
            out_optimized_graph_buf.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
