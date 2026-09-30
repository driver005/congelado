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

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_FilesystemOps{
            .struct_size = TF_FILESYSTEM_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TF_FilesystemOps>{&TF_FilesystemOps::from_handle(plugin_context)};
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_FilesystemOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },

        };
    }

    const ::TF_FilesystemOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Filesystem& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_FilesystemOps m_vtable;
    TF_Filesystem m_handle;
};

} // namespace ice::builder
