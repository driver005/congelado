// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/writable_file.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/writable_file.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_filesystem_sonic:writable_file;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TF_WritableFileOps : public ice::sonic::Runtime<::TF_WritableFileOps, ::TF_WritableFile>
{
public:
    TF_WritableFileOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_WritableFileOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_WritableFile* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_WritableFileOps(const ::TF_WritableFileOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_WritableFileOps(const ::TF_WritableFileOps* ops, ::TF_WritableFile* handle) noexcept :
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

    void append(
        const ice::sonic::String& buffer,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->append(get_handle(), buffer.get_handle(), out_status.get_handle());
    }

    void tell(int64_t* out_position, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->tell(get_handle(), out_position, out_status.get_handle());
    }

    void flush(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->flush(get_handle(), out_status.get_handle());
    }

    void sync(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->sync(get_handle(), out_status.get_handle());
    }

    void close(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->close(get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
