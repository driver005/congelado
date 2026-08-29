module;

#include "c/extern/filesystem/filesystem.h"

export module cc_abi_sonic_filesystem:leaves.random_access_file;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_builder_filesystem;

export namespace ice::sonic {

class RandomAccessFile : public ice::builder::RandomAccessFile
{
public:
    ~RandomAccessFile() { if (m_handle && m_ops) m_ops->random_access_file__destroy(m_handle); }

    RandomAccessFile(const RandomAccessFile&) = delete;
    RandomAccessFile& operator=(const RandomAccessFile&) = delete;
    RandomAccessFile(RandomAccessFile&&) = delete;
    RandomAccessFile& operator=(RandomAccessFile&&) = delete;

    explicit RandomAccessFile(TF_Filesystem* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    std::expected<std::int64_t, ice::Status>
    read(std::uint64_t offset, std::size_t n, char* buffer) override
    {
        ice::Status status;
        std::int64_t result =
            m_ops->random_access_file__read(m_handle, offset, n, buffer, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

private:
    TF_Filesystem* m_ops; void* m_handle;
};

} // namespace ice::sonic
