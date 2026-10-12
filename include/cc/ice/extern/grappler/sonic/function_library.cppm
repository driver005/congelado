// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/function_library.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/function_library.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_grappler_sonic:function_library;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class GrapplerFunctionLibrary :
    public ice::sonic::Runtime<::TFGrapplerFunctionLibraryOps, ::TFGrapplerFunctionLibrary>
{
public:
    GrapplerFunctionLibrary(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    GrapplerFunctionLibrary(
        const ice::sonic::Registration& registry,
        ::TFGrapplerFunctionLibrary* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            handle,
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    explicit GrapplerFunctionLibrary(const ::TFGrapplerFunctionLibraryOps* ops) noexcept :
        Runtime(ops)
    {
    }

    GrapplerFunctionLibrary(
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
        const ice::sonic::Buffer& out_buf,
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
