// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/filesystem.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/filesystem.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_filesystem_sonic:filesystem;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_FilesystemOps : public ice::sonic::Runtime<::TF_FilesystemOps, ::TF_Filesystem>
{
public:
    TF_FilesystemOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_FilesystemOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Filesystem* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_FilesystemOps(const ::TF_FilesystemOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_FilesystemOps(const ::TF_FilesystemOps* ops, ::TF_Filesystem* handle) noexcept :
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

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }
};

} // namespace ice::sonic
