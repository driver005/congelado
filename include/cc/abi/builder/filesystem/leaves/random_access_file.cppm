export module cc_abi_builder_filesystem:leaves.random_access_file;

import std;
import cc_abi_primitives;

export namespace ice::builder {

class RandomAccessFile
{
public:
    virtual ~RandomAccessFile() = default;

    virtual std::expected<std::int64_t, ice::Status>
    read(std::uint64_t offset, std::size_t n, char* buffer) = 0;
};

} // namespace ice::builder
