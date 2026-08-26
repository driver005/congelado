module;

#include "c/extern/filesystem.h"

export module cc_abi_builder_filesystem:random_access_file_builder;

export namespace ice::builder {

class RandomAccessFileBuilder
{
public:
    RandomAccessFileBuilder() :
        m_handle{TF_RandomAccessFileNew()}
    {
    }

    ~RandomAccessFileBuilder()
    {
        TF_RandomAccessFileDelete(m_handle);
    }

    RandomAccessFileBuilder(const RandomAccessFileBuilder&) = delete;
    RandomAccessFileBuilder& operator=(const RandomAccessFileBuilder&) = delete;

    RandomAccessFileBuilder(RandomAccessFileBuilder&& other) noexcept :
        m_handle(other.m_handle)
    {

        other.m_handle = nullptr;
    }

    RandomAccessFileBuilder& operator=(RandomAccessFileBuilder&& other) noexcept
    {

        if (this != &other) {
            TF_RandomAccessFileDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    void* get_plugin_file() const
    {
        return m_handle->plugin_file;
    }

    RandomAccessFileBuilder& set_plugin_file(void* value)
    {

        TF_RandomAccessFile_SetPluginFile(m_handle, value);
        return *this;
    }

    // Underlying handle — pass directly to the C ABI
    TF_RandomAccessFile* get_handle()
    {
        return m_handle;
    }

    const TF_RandomAccessFile* get_handle() const
    {
        return m_handle;
    }

private:
    TF_RandomAccessFile* m_handle;
};

} // namespace ice::builder
