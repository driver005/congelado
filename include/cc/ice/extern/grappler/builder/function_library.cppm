// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/function_library.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/function_library.h"

export module cc_ice_extern_grappler_builder:function_library;

import std;

export namespace ice::builder {

class TFGrapplerFunctionLibraryOps
{
public:
    TFGrapplerFunctionLibraryOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFGrapplerFunctionLibraryOps(const TFGrapplerFunctionLibraryOps&) = delete;
    TFGrapplerFunctionLibraryOps& operator=(const TFGrapplerFunctionLibraryOps&) = delete;

    static TFGrapplerFunctionLibraryOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGrapplerFunctionLibraryOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerFunctionLibraryOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGrapplerFunctionLibraryOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerFunctionLibraryOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> look_up_op_def(
        const ice::sonic::String& name,
        const ice::sonic::TF_BufferOps& out_buf
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGrapplerFunctionLibraryOps{
            .struct_size = TF_RAPPLERFUNCTIONLIBRARY_STRUCT_SIZE,
            .look_up_op_def = [](TFGrapplerFunctionLibrary* lib,
                                 const TF_String* name,
                                 TF_Buffer* out_buf,
                                 TF_Status* out_status) noexcept
            {
                auto res = TFGrapplerFunctionLibraryOps::from_handle(lib).look_up_op_def(
                    ice::sonic::String::wrap(name),
                    ice::sonic::TF_BufferOps::wrap(out_buf)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFGrapplerFunctionLibraryOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFGrapplerFunctionLibrary& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGrapplerFunctionLibraryOps m_vtable;
    TFGrapplerFunctionLibrary m_handle;
};

} // namespace ice::builder
