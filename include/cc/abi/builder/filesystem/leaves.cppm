module;

#include "c/extern/filesystem/option_types.h"

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

    // Range-first: the C ABI carries (char*, size_t) — the C++ interface takes a span
    // (the vtable adapter is the only place the raw pair exists).
    [[nodiscard]] virtual std::expected<std::int64_t, ice::Status>
    read(std::uint64_t offset, std::span<char> buffer) noexcept = 0;
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

    [[nodiscard]] virtual std::expected<void, ice::Status>
    append(const ice::String& buffer) noexcept = 0;
    [[nodiscard]] virtual std::expected<std::int64_t, ice::Status> tell() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> flush() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> sync() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> close() noexcept = 0;
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

    // Range-first: data + length are one span instead of two split accessors; the
    // vtable adapter derives the C ABI's __data/__length pair from it.
    virtual std::span<const std::byte> data() noexcept = 0;
};

// FilesystemOption — thin value wrapper over the C TF_Filesystem_Option struct, so
// option values cross the builder tier by value without leaking the raw struct.
// Every member is noexcept; the wrapped struct is trivially copyable, so the
// defaulted copy ctor / assignment are correct.
class FilesystemOption
{
public:
    FilesystemOption() noexcept = default;

    explicit FilesystemOption(const TF_Filesystem_Option& option) noexcept :
        m_option{option}
    {
    }

    TF_Filesystem_Option to_c() const noexcept
    {
        return m_option;
    }

    static FilesystemOption create(const TF_Filesystem_Option& option) noexcept
    {
        return FilesystemOption{option};
    }

    TF_Filesystem_Option_Type get_type_tag() const noexcept
    {
        return m_option.type_tag;
    }

    const char* get_name() const noexcept
    {
        return m_option.name;
    }

    const char* get_description() const noexcept
    {
        return m_option.description;
    }

    TF_Filesystem_Option_Value get_value() const noexcept
    {
        return m_option.value;
    }

    const TF_Filesystem_Option* get_handle() const noexcept
    {
        return &m_option;
    }

    TF_Filesystem_Option* get_handle() noexcept
    {
        return &m_option;
    }

private:
    TF_Filesystem_Option m_option{};
};

// delete_recursively's out-params, collected into one value the caller passes in.
struct DeleteRecursivelyResult
{
    std::uint64_t undeleted_files;
    std::uint64_t undeleted_dirs;
};

} // namespace ice::builder
