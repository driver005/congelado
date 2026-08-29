module;

#include "c/extern/filesystem/filesystem.h"

export module cc_abi_sonic_filesystem:leaves.writable_file;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_builder_filesystem;

export namespace ice::sonic {

class WritableFile : public ice::builder::WritableFile
{
public:
    ~WritableFile() { if (m_handle && m_ops) m_ops->writable_file__destroy(m_handle); }

    WritableFile(const WritableFile&) = delete;
    WritableFile& operator=(const WritableFile&) = delete;
    WritableFile(WritableFile&&) = delete;
    WritableFile& operator=(WritableFile&&) = delete;

    explicit WritableFile(TF_Filesystem* ops, void* handle) : m_ops{ops}, m_handle{handle} {}

    std::expected<void, ice::Status> append(const ice::String& buffer) override
    {
        ice::Status status;
        m_ops->writable_file__append(m_handle, buffer.get_handle(), buffer.size(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<std::int64_t, ice::Status> tell() override
    {
        ice::Status status;
        std::int64_t result = m_ops->writable_file__tell(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    std::expected<void, ice::Status> flush() override
    {
        ice::Status status;
        m_ops->writable_file__flush(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> sync() override
    {
        ice::Status status;
        m_ops->writable_file__sync(m_handle, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> close() override
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

} // namespace ice::sonic
