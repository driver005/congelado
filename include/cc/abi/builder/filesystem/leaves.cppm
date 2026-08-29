export module cc_abi_builder_filesystem:leaves;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder::filesystem {

// RandomAccessFile/WritableFile/ReadOnlyMemoryRegion — produced by Filesystem's own
// new_random_access_file/new_writable_file/new_appendable_file/
// new_read_only_memory_region_from_file, no independent existence outside their producing
// Filesystem (mirrors ice::builder::Function's relationship to
// Builder::enter_border_patrol).

class RandomAccessFile
{
public:
    virtual ~RandomAccessFile() = default;

    // Reads up to n bytes starting at offset into buffer (caller-owned, at least n bytes).
    // Returns bytes actually read; a short read due to EOF is still signaled via Status (same
    // contract the underlying C ABI always used — TF_OUT_OF_RANGE for EOF).
    virtual std::expected<std::int64_t, ice::Status>
    read(std::uint64_t offset, std::size_t n, char* buffer) = 0;
};

class WritableFile
{
public:
    virtual ~WritableFile() = default;

    virtual std::expected<void, ice::Status> append(const char* buffer, std::size_t n) = 0;
    virtual std::expected<std::int64_t, ice::Status> tell() = 0;
    virtual std::expected<void, ice::Status> flush() = 0;
    virtual std::expected<void, ice::Status> sync() = 0;
    virtual std::expected<void, ice::Status> close() = 0;
};

class ReadOnlyMemoryRegion
{
public:
    virtual ~ReadOnlyMemoryRegion() = default;

    virtual const void* data() = 0;
    virtual std::uint64_t length() = 0;
};

} // namespace ice::builder
