// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/random_access_file.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/random_access_file.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_filesystem_sonic:random_access_file;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class RandomAccessFile : public ice::sonic::Runtime<::TF_RandomAccessFileOps, ::TF_RandomAccessFile>
{
public:
    RandomAccessFile(
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

    RandomAccessFile(
        const ice::sonic::Registration& registry,
        ::TF_RandomAccessFile* handle,
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

    explicit RandomAccessFile(const ::TF_RandomAccessFileOps* ops) noexcept :
        Runtime(ops)
    {
    }

    RandomAccessFile(const ::TF_RandomAccessFileOps* ops, ::TF_RandomAccessFile* handle) noexcept :
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

    void read(
        uint64_t offset,
        size_t n,
        char* buffer,
        std::int64_t& out_bytes_read,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->read(get_handle(), offset, n, buffer, &out_bytes_read, out_status.get_handle());
    }
};

} // namespace ice::sonic
