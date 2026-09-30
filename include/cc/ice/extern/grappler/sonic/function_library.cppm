// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/function_library.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/function_library.h"

export module cc_ice_extern_grappler_sonic:function_library;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGrapplerFunctionLibraryOps :
    public ice::sonic::Runtime<TFGrapplerFunctionLibraryOps, TFGrapplerFunctionLibraryOps>
{
public:
    explicit TFGrapplerFunctionLibraryOps(
        TFGrapplerFunctionLibraryOps* ops,
        void* plugin_context
    ) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "grappler";

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    look_up_op_def(const ice::sonic::String& name, const ice::sonic::TF_BufferOps& out_buf) noexcept
    {
        ice::sonic::Status status;
        m_ops->look_up_op_def(
            get_handle(),
            name.get_handle(),
            out_buf.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
