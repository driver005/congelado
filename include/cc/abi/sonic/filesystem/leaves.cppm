module;

#include "c/extern/filesystem/filesystem.h"

export module cc_abi_sonic_filesystem:leaves;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
export namespace ice::sonic::filesystem {

// RandomAccessFileRuntime/WritableFileRuntime/ReadOnlyMemoryRegionRuntime — cross-plugin C ABI handle, produced only by the parent runtime's factory methods.

class RandomAccessFileRuntime : public ice::builder::RandomAccessFile
{
public:
    ~RandomAccessFileRuntime() { if (m_handle && m_ops) m_ops->random_access_file__destroy(m_handle); }

    RandomAccessFileRuntime(const RandomAccessFileRuntime&) = delete;
    RandomAccessFileRuntime& operator=(const RandomAccessFileRuntime&) = delete;
    RandomAccessFileRuntime(RandomAccessFileRuntime&&) = delete;
    RandomAccessFileRuntime& operator=(RandomAccessFileRuntime&&) = delete;

    explicit RandomAccessFileRuntime(TF_Filesystem* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    std::expected<std::int64_t, ice::Status>
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
    TF_Filesystem* m_ops; void* m_handle;
};

class WritableFileRuntime : public ice::builder::WritableFile
{
public:
    ~WritableFileRuntime() { if (m_handle && m_ops) m_ops->writable_file__destroy(m_handle); }

    WritableFileRuntime(const WritableFileRuntime&) = delete;
    WritableFileRuntime& operator=(const WritableFileRuntime&) = delete;
    WritableFileRuntime(WritableFileRuntime&&) = delete;
    WritableFileRuntime& operator=(WritableFileRuntime&&) = delete;

    explicit WritableFileRuntime(TF_Filesystem* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    std::expected<void, ice::Status> append(const char* buffer, std::size_t n)
    {


        ice::Status status;
        ice::String sr_buffer(std::string(buffer, n));
        m_ops->writable_file__append(m_handle, sr_buffer.get_handle(), n, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<std::int64_t, ice::Status> tell()
    {


        ice::Status status;
        std::int64_t result = m_ops->writable_file__tell(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    std::expected<void, ice::Status> flush()
    {


        ice::Status status;
        m_ops->writable_file__flush(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> sync()
    {


        ice::Status status;
        m_ops->writable_file__sync(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> close()
    {


        ice::Status status;
        m_ops->writable_file__close(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_Filesystem* m_ops; void* m_handle;
};

class ReadOnlyMemoryRegionRuntime : public ice::builder::ReadOnlyMemoryRegion
{
public:
    ~ReadOnlyMemoryRegionRuntime() { if (m_handle && m_ops) m_ops->read_only_memory_region__destroy(m_handle); }

    ReadOnlyMemoryRegionRuntime(const ReadOnlyMemoryRegionRuntime&) = delete;
    ReadOnlyMemoryRegionRuntime& operator=(const ReadOnlyMemoryRegionRuntime&) = delete;
    ReadOnlyMemoryRegionRuntime(ReadOnlyMemoryRegionRuntime&&) = delete;
    ReadOnlyMemoryRegionRuntime& operator=(ReadOnlyMemoryRegionRuntime&&) = delete;

    explicit ReadOnlyMemoryRegionRuntime(TF_Filesystem* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    const void* data()
    {


        return m_ops->read_only_memory_region__data(m_handle);
    }

    std::uint64_t length()
    {


        return m_ops->read_only_memory_region__length(m_handle);
    }

private:
    TF_Filesystem* m_ops; void* m_handle;
};

} // namespace ice::sonic
