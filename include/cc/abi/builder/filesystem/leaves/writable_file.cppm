export module cc_abi_builder_filesystem:leaves.writable_file;

import std;
import cc_abi_primitives;

export namespace ice::builder {

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

} // namespace ice::builder
