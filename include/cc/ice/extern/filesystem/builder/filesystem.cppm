// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/filesystem.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/filesystem.h"

export module cc_ice_extern_filesystem_builder:filesystem;

import std;

export namespace ice::builder {

class TF_FilesystemOps
{
public:
    TF_FilesystemOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_FilesystemOps(const TF_FilesystemOps&) = delete;
    TF_FilesystemOps& operator=(const TF_FilesystemOps&) = delete;

    static TF_FilesystemOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_FilesystemOps*>(ctx);
    }

    template<typename HandleT>
    static TF_FilesystemOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_FilesystemOps*>(handle->plugin_data);
    }

    virtual ~TF_FilesystemOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_FilesystemOps{
            .struct_size = TF_FILESYSTEM_STRUCT_SIZE,
            .destroy =
                [](TF_Filesystem* filesystem) noexcept
            {
                TF_FilesystemOps::from_handle(filesystem).destroy();
            },
            .get_name =
                [](TF_Filesystem* filesystem, TF_String* out_name) noexcept
            {
                TF_FilesystemOps::from_handle(filesystem)
                    .get_name(ice::sonic::String::wrap(out_name));
            },

        };
    }

    const ::TF_FilesystemOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Filesystem& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_FilesystemOps m_vtable;
    TF_Filesystem m_handle;
};

} // namespace ice::builder
