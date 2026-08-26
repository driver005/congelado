module;

#include "c/extern/filesystem.h"

export module cc_abi_builder_filesystem:filesystem_plugin_builder;

export namespace ice::builder {

// FilesystemPluginBuilder wrapper (owned pointer)
class FilesystemPluginBuilder
{
public:
    FilesystemPluginBuilder() :
        m_handle{TF_FilesystemPluginInfoNew()}
    {
    }

    ~FilesystemPluginBuilder()
    {
        TF_FilesystemPluginInfoDelete(m_handle);
    }

    FilesystemPluginBuilder(const FilesystemPluginBuilder&) = delete;
    FilesystemPluginBuilder& operator=(const FilesystemPluginBuilder&) = delete;

    FilesystemPluginBuilder(FilesystemPluginBuilder&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    FilesystemPluginBuilder& operator=(FilesystemPluginBuilder&& other) noexcept
    {

        if (this != &other) {
            TF_FilesystemPluginInfoDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    FilesystemPluginBuilder& set_num_schemes(size_t num)
    {

        TF_FilesystemPluginInfo_SetNumSchemes(m_handle, num);
        return *this;
    }

    FilesystemPluginBuilder& set_ops(TF_FilesystemPluginOps* ops)
    {

        TF_FilesystemPluginInfo_SetOps(m_handle, ops);
        return *this;
    }

    FilesystemPluginBuilder& set_memory_allocator(
        TP_FilesystemPlugin_MemoryAllocate alloc, TP_FilesystemPlugin_MemoryFree dealloc
    )
    {

        TF_FilesystemPluginInfo_SetMemoryAllocator(m_handle, alloc, dealloc);
        return *this;
    }

    // Underlying handle — pass directly to the C ABI
    TF_FilesystemPluginInfo* get_handle()
    {
        return m_handle;
    }

    const TF_FilesystemPluginInfo* get_handle() const
    {
        return m_handle;
    }

private:
    TF_FilesystemPluginInfo* m_handle;
};

} // namespace ice::builder
