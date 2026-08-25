module;

#include "c/extern/filesystem.h"

export module cc_abi_builder_filesystem:filesystem_plugin_ops_builder;

export namespace ice {

// FilesystemPluginOpsBuilder wrapper (owned pointer)
class FilesystemPluginOpsBuilder {
  public:
    FilesystemPluginOpsBuilder() : m_handle{TF_FilesystemPluginOpsNew()} {}
    ~FilesystemPluginOpsBuilder() { TF_FilesystemPluginOpsDelete(m_handle); }

    FilesystemPluginOpsBuilder(const FilesystemPluginOpsBuilder &) = delete;
    FilesystemPluginOpsBuilder &operator=(const FilesystemPluginOpsBuilder &) = delete;

    FilesystemPluginOpsBuilder(FilesystemPluginOpsBuilder &&other) noexcept : m_handle{other.m_handle} {
        other.m_handle = nullptr;
    }
    FilesystemPluginOpsBuilder &operator=(FilesystemPluginOpsBuilder &&other) noexcept {

        if (this != &other) {
            TF_FilesystemPluginOpsDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;

    }

    FilesystemPluginOpsBuilder &set_filesystem_ops(TF_FilesystemOps *ops) {

        TF_FilesystemPluginOps_SetFilesystemOps(m_handle, ops);
        return *this;

    }
    FilesystemPluginOpsBuilder &set_random_access_file_ops(TF_RandomAccessFileOps *ops) {

        TF_FilesystemPluginOps_SetRandomAccessFileOps(m_handle, ops);
        return *this;

    }
    FilesystemPluginOpsBuilder &set_writable_file_ops(TF_WritableFileOps *ops) {

        TF_FilesystemPluginOps_SetWritableFileOps(m_handle, ops);
        return *this;

    }
    FilesystemPluginOpsBuilder &set_read_only_memory_region_ops(TF_ReadOnlyMemoryRegionOps *ops) {

        TF_FilesystemPluginOps_SetReadOnlyMemoryRegionOps(m_handle, ops);
        return *this;

    }
    void set_version_metadata() { TF_SetFilesystemVersionMetadata(m_handle); }

    // Underlying handle — pass directly to the C ABI
    TF_FilesystemPluginOps *get_handle() { return m_handle; }
    const TF_FilesystemPluginOps *get_handle() const { return m_handle; }

  private:
    TF_FilesystemPluginOps *m_handle;
};

} // namespace ice
