// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/random_access_file.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/random_access_file.h"

export module cc_abi_sonic_filesystem;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_RandomAccessFileOps :
    public ice::sonic::Runtime<TF_RandomAccessFileOps, TF_RandomAccessFileOps>
{
public:
    explicit TF_RandomAccessFileOps(TF_RandomAccessFileOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "filesystem";

    [[nodiscard]] std::expected<void, ice::Status>
    read(uint64_t offset, size_t n, char* buffer, int64_t* out_bytes_read) noexcept
    {
        ice::Status status;
        m_ops->read(get_handle(), offset, n, buffer, out_bytes_read status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
