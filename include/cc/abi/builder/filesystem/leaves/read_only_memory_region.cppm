export module cc_abi_builder_filesystem:leaves.read_only_memory_region;

import std;
import cc_abi_primitives;

export namespace ice::builder {

class ReadOnlyMemoryRegion
{
public:
    virtual ~ReadOnlyMemoryRegion() = default;

    virtual const void* data() = 0;
    virtual std::uint64_t length() = 0;
};

} // namespace ice::builder
