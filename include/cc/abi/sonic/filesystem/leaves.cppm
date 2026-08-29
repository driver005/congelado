module;

#include "c/extern/filesystem/filesystem.h"

export module cc_abi_sonic_filesystem:leaves;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;

export namespace ice::sonic {

class RandomAccessFile
{
public:
    ~RandomAccessFile()
    {
        if (m_ops && m_handle) {
            m_ops->random_access_file__destroy(m_handle);
        }
    }

    RandomAccessFile(const RandomAccessFile&) = delete;
    RandomAccessFile& operator=(const RandomAccessFile&) = delete;
    RandomAccessFile(RandomAccessFile&&) = delete;
    RandomAccessFile& operator=(RandomAccessFile&&) = delete;

    explicit RandomAccessFile(TF_Filesystem* ops, void* handle) :
        m_ops{ops},
        m_handle{handle}
    {
    }

    [[nodiscard]] std::expected<std::int64_t, ice::Status>
    read(std::uint64_t offset, std::size_t n, char* buffer)
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
    TF_Filesystem* m_ops;
    void* m_handle;
};

class WritableFile
{
public:
    ~WritableFile()
    {
        if (m_ops && m_handle) {
            m_ops->writable_file__destroy(m_handle);
        }
    }

    WritableFile(const WritableFile&) = delete;
    WritableFile& operator=(const WritableFile&) = delete;
    WritableFile(WritableFile&&) = delete;
    WritableFile& operator=(WritableFile&&) = delete;

    explicit WritableFile(TF_Filesystem* ops, void* handle) :
        m_ops{ops},
        m_handle{handle}
    {
    }

    [[nodiscard]] std::expected<void, ice::Status> append(const ice::String& buffer)
    {
        ice::Status status;
        m_ops->writable_file__append(
            m_handle,
            buffer.get_handle(),
            buffer.size(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<std::int64_t, ice::Status> tell()
    {
        ice::Status status;
        std::int64_t result = m_ops->writable_file__tell(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    [[nodiscard]] std::expected<void, ice::Status> flush()
    {
        ice::Status status;
        m_ops->writable_file__flush(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> sync()
    {
        ice::Status status;
        m_ops->writable_file__sync(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> close()
    {
        ice::Status status;
        m_ops->writable_file__close(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_Filesystem* m_ops;
    void* m_handle;
};

class ReadOnlyMemoryRegion
{
public:
    ~ReadOnlyMemoryRegion()
    {
        if (m_ops && m_handle) {
            m_ops->read_only_memory_region__destroy(m_handle);
        }
    }

    ReadOnlyMemoryRegion(const ReadOnlyMemoryRegion&) = delete;
    ReadOnlyMemoryRegion& operator=(const ReadOnlyMemoryRegion&) = delete;
    ReadOnlyMemoryRegion(ReadOnlyMemoryRegion&&) = delete;
    ReadOnlyMemoryRegion& operator=(ReadOnlyMemoryRegion&&) = delete;

    explicit ReadOnlyMemoryRegion(TF_Filesystem* ops, void* handle) :
        m_ops{ops},
        m_handle{handle}
    {
    }

    const void* data()
    {
        return m_ops->read_only_memory_region__data(m_handle);
    }

    std::uint64_t length()
    {
        return m_ops->read_only_memory_region__length(m_handle);
    }

private:
    TF_Filesystem* m_ops;
    void* m_handle;
};

} // namespace ice::sonic
