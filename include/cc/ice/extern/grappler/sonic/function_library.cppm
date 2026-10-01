// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/function_library.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/function_library.h"

export module cc_ice_extern_grappler_sonic:function_library;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGrapplerFunctionLibraryOps :
    public ice::sonic::Runtime<::TFGrapplerFunctionLibraryOps, ::TFGrapplerFunctionLibrary>
{
public:
    template<typename Registry>
    TFGrapplerFunctionLibraryOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFGrapplerFunctionLibraryOps(
        Registry& registry,
        ::TFGrapplerFunctionLibrary* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFGrapplerFunctionLibraryOps(const ::TFGrapplerFunctionLibraryOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGrapplerFunctionLibraryOps(
        const ::TFGrapplerFunctionLibraryOps* ops,
        ::TFGrapplerFunctionLibrary* handle
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

    void look_up_op_def(
        const ice::sonic::String& name,
        const ice::sonic::TF_BufferOps& out_buf,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->look_up_op_def(
            get_handle(),
            name.get_handle(),
            out_buf.get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
