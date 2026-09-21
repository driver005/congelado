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
    static TFGrapplerFunctionLibraryOps* create(void* ctx) noexcept
    {
        return static_cast<TFGrapplerFunctionLibraryOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerFunctionLibraryOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGrapplerFunctionLibraryOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerFunctionLibraryOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    look_up_op_def(const char* name, const ice::sonic::TF_BufferOps& out_buf) noexcept = 0;

    static TFGrapplerFunctionLibraryOps* get_generic_vtable()
    {
        static TFGrapplerFunctionLibraryOps vtable = {
            .struct_size = TF_RAPPLERFUNCTIONLIBRARY_STRUCT_SIZE,
            .look_up_op_def = [](TFGrapplerFunctionLibrary* lib,
                                 const char* name,
                                 TF_Buffer* out_buf,
                                 TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerFunctionLibraryOps::create(lib);
                auto res = self->look_up_op_def(name, ice::sonic::TF_BufferOps::wrap(out_buf));
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
