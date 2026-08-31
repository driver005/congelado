module;

#include "c/extern/filesystem/filesystem.h"

export module cc_abi_sonic_filesystem:leaves;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;

export namespace ice::sonic {

// Host-side RAII wrappers over plugin-allocated file handles: each owns the C
// destroy call in its destructor (move-only, like the plugin-side leaves).
// Every member is noexcept — the std::expected return is the only failure channel.

class RandomAccessFile
{
public:
    ~RandomAccessFile()
    {
        if (m_ops && m_handle) {
            m_ops->random_access_file_destroy(m_handle);
        }
    }

    RandomAccessFile(const RandomAccessFile&) = delete;
    RandomAccessFile& operator=(const RandomAccessFile&) = delete;
    RandomAccessFile(RandomAccessFile&&) = delete;
    RandomAccessFile& operator=(RandomAccessFile&&) = delete;

    explicit RandomAccessFile(TF_Filesystem* ops, TF_RandomAccessFile* handle) noexcept :
        m_ops{ops},
        m_handle{handle}
    {
    }

    // Range-first: the C vtable carries (char*, size_t) — this C++ interface takes a span
    // and adapts at the single boundary line below.
    [[nodiscard]] std::expected<std::int64_t, ice::Status>
    read(std::uint64_t offset, std::span<char> buffer) noexcept
    {
        ice::Status status;
        std::int64_t result = m_ops->random_access_file_read(
            m_handle,
            offset,
            buffer.size(),
            buffer.data(),
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

private:
    TF_Filesystem* m_ops;
    TF_RandomAccessFile* m_handle;
};

class WritableFile
{
public:
    ~WritableFile()
    {
        if (m_ops && m_handle) {
            m_ops->writable_file_destroy(m_handle);
        }
    }

    WritableFile(const WritableFile&) = delete;
    WritableFile& operator=(const WritableFile&) = delete;
    WritableFile(WritableFile&&) = delete;
    WritableFile& operator=(WritableFile&&) = delete;

    explicit WritableFile(TF_Filesystem* ops, TF_WritableFile* handle) noexcept :
        m_ops{ops},
        m_handle{handle}
    {
    }

    [[nodiscard]] std::expected<void, ice::Status> append(const ice::String& buffer) noexcept
    {
        ice::Status status;
        m_ops->writable_file_append(m_handle, buffer.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<std::int64_t, ice::Status> tell() noexcept
    {
        ice::Status status;
        std::int64_t result = m_ops->writable_file_tell(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    [[nodiscard]] std::expected<void, ice::Status> flush() noexcept
    {
        ice::Status status;
        m_ops->writable_file_flush(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> sync() noexcept
    {
        ice::Status status;
        m_ops->writable_file_sync(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> close() noexcept
    {
        ice::Status status;
        m_ops->writable_file_close(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

private:
    TF_Filesystem* m_ops;
    TF_WritableFile* m_handle;
};

class ReadOnlyMemoryRegion
{
public:
    ~ReadOnlyMemoryRegion()
    {
        if (m_ops && m_handle) {
            m_ops->read_only_memory_region_destroy(m_handle);
        }
    }

    ReadOnlyMemoryRegion(const ReadOnlyMemoryRegion&) = delete;
    ReadOnlyMemoryRegion& operator=(const ReadOnlyMemoryRegion&) = delete;
    ReadOnlyMemoryRegion(ReadOnlyMemoryRegion&&) = delete;
    ReadOnlyMemoryRegion& operator=(ReadOnlyMemoryRegion&&) = delete;

    explicit ReadOnlyMemoryRegion(TF_Filesystem* ops, TF_ReadOnlyMemoryRegion* handle) noexcept :
        m_ops{ops},
        m_handle{handle}
    {
    }

    // Range-first: data + length are one span instead of two split C ABI calls.
    std::span<const std::byte> data() noexcept
    {
        const auto* raw = static_cast<const std::byte*>(m_ops->read_only_memory_region_data(m_handle));
        return {raw, static_cast<size_t>(m_ops->read_only_memory_region_length(m_handle))};
    }

private:
    TF_Filesystem* m_ops;
    TF_ReadOnlyMemoryRegion* m_handle;
};

} // namespace ice::sonic
