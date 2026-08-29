export module cc_abi_builder_filesystem:leaves;

import std;
import cc_abi_primitives;

export namespace ice::builder {

class RandomAccessFile
{
public:
    // Recover the RandomAccessFile instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static RandomAccessFile* create(void* ctx) noexcept
    {
        return static_cast<RandomAccessFile*>(ctx);
    }

    virtual ~RandomAccessFile() = default;

    virtual [[nodiscard]] std::expected<std::int64_t, ice::Status>
    read(std::uint64_t offset, std::size_t n, char* buffer) = 0;
};

class WritableFile
{
public:
    // Recover the WritableFile instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static WritableFile* create(void* ctx) noexcept
    {
        return static_cast<WritableFile*>(ctx);
    }

    virtual ~WritableFile() = default;

    virtual [[nodiscard]] std::expected<void, ice::Status> append(const ice::String& buffer) = 0;
    virtual [[nodiscard]] std::expected<std::int64_t, ice::Status> tell() = 0;
    virtual [[nodiscard]] std::expected<void, ice::Status> flush() = 0;
    virtual [[nodiscard]] std::expected<void, ice::Status> sync() = 0;
    virtual [[nodiscard]] std::expected<void, ice::Status> close() = 0;
};

class ReadOnlyMemoryRegion
{
public:
    // Recover the ReadOnlyMemoryRegion instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static ReadOnlyMemoryRegion* create(void* ctx) noexcept
    {
        return static_cast<ReadOnlyMemoryRegion*>(ctx);
    }

    virtual ~ReadOnlyMemoryRegion() = default;

    virtual const void* data() = 0;
    virtual std::uint64_t length() = 0;
};

} // namespace ice::builder
