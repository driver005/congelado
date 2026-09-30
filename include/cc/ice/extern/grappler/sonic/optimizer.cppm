// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/optimizer.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/optimizer.h"

export module cc_ice_extern_grappler_sonic:optimizer;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGrapplerOptimizerOps :
    public ice::sonic::Runtime<TFGrapplerOptimizerOps, TFGrapplerOptimizerOps>
{
public:
    explicit TFGrapplerOptimizerOps(TFGrapplerOptimizerOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "grappler";

    [[nodiscard]] std::expected<void, ice::sonic::Status> optimize(
        const ice::sonic::TF_BufferOps& graph_buf,
        const ice::sonic::TFGrapplerItemOps& item,
        const ice::sonic::TF_BufferOps& out_optimized_graph_buf
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->optimize(
            get_handle(),
            graph_buf.get_handle(),
            item.get_handle(),
            out_optimized_graph_buf.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
