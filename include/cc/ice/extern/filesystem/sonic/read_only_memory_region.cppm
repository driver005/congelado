// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/filesystem/read_only_memory_region.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/filesystem/read_only_memory_region.h"

export module cc_ice_extern_filesystem_sonic:read_only_memory_region;

import std;
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

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void data(const void** out_data) noexcept
    {
        m_ops->data(get_handle(), out_data);
    }

    void length(uint64_t* out_length) noexcept
    {
        m_ops->length(get_handle(), out_length);
    }
};

} // namespace ice::sonic
