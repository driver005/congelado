module;

#include "c/extern/filesystem/filesystem.h"

export module cc_abi_sonic_filesystem:leaves.read_only_memory_region;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_builder_filesystem;

export namespace ice::sonic {

class ReadOnlyMemoryRegion : public ice::builder::ReadOnlyMemoryRegion
{
public:
    ~ReadOnlyMemoryRegion() { if (m_handle && m_ops) m_ops->read_only_memory_region__destroy(m_handle); }

    ReadOnlyMemoryRegion(const ReadOnlyMemoryRegion&) = delete;
    ReadOnlyMemoryRegion& operator=(const ReadOnlyMemoryRegion&) = delete;
    ReadOnlyMemoryRegion(ReadOnlyMemoryRegion&&) = delete;
    ReadOnlyMemoryRegion& operator=(ReadOnlyMemoryRegion&&) = delete;

    explicit ReadOnlyMemoryRegion(TF_Filesystem* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    const void* data() override
    {
        return m_ops->read_only_memory_region__data(m_handle);
    }

    std::uint64_t length() override
    {
        return m_ops->read_only_memory_region__length(m_handle);
    }

private:
    TF_Filesystem* m_ops; void* m_handle;
};

} // namespace ice::sonic
