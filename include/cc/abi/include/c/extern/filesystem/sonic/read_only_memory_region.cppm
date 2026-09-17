// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/read_only_memory_region.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/read_only_memory_region.h"

export module cc_abi_sonic_filesystem;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ReadOnlyMemoryRegionOps :
    public ice::sonic::Runtime<TF_ReadOnlyMemoryRegionOps, TF_ReadOnlyMemoryRegionOps>
{
public:
    explicit TF_ReadOnlyMemoryRegionOps(
        TF_ReadOnlyMemoryRegionOps* ops,
        void* plugin_context
    ) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "filesystem";

    [[nodiscard]] std::expected<void, ice::Status> data(const void** out_data) noexcept
    {
        ice::Status status;
        m_ops->data(get_handle(), out_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> length(uint64_t* out_length) noexcept
    {
        ice::Status status;
        m_ops->length(get_handle(), out_length, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
