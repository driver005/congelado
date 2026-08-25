module;

#include "c/extern/filesystem.h"

export module cc_abi_builder_filesystem:filesystem_ops;

import cc_abi_builder_intern;

import :random_access_file;
import :writable_file;
import :read_only_memory_region;

export namespace ice {

// FilesystemOpsBuilder wrapper (owned pointer)
class FilesystemOpsBuilder {
  public:
    FilesystemOpsBuilder() : m_handle{TF_FilesystemOpsNew()} {}
    ~FilesystemOpsBuilder() { TF_FilesystemOpsDelete(m_handle); }

    FilesystemOpsBuilder(const FilesystemOpsBuilder &) = delete;
    FilesystemOpsBuilder &operator=(const FilesystemOpsBuilder &) = delete;

    FilesystemOpsBuilder(FilesystemOpsBuilder &&other) noexcept : m_handle{other.m_handle} {
        other.m_handle = nullptr;
    }
    FilesystemOpsBuilder &operator=(FilesystemOpsBuilder &&other) noexcept {
        if (this != &other) {
            TF_FilesystemOpsDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    FilesystemOpsBuilder &set_init(TP_Filesystem_Init callback) {
        TF_FilesystemOps_SetInit(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_cleanup(TP_Filesystem_Cleanup callback) {
        TF_FilesystemOps_SetCleanup(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_new_random_access_file(TP_Filesystem_NewRandomAccessFile callback) {
        TF_FilesystemOps_SetNewRandomAccessFile(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_new_writable_file(TP_Filesystem_NewWritableFile callback) {
        TF_FilesystemOps_SetNewWritableFile(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_new_appendable_file(TP_Filesystem_NewAppendableFile callback) {
        TF_FilesystemOps_SetNewAppendableFile(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_new_read_only_memory_region_from_file(
        TP_Filesystem_NewReadOnlyMemoryRegionFromFile callback) {
        TF_FilesystemOps_SetNewReadOnlyMemoryRegionFromFile(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_create_dir(TP_Filesystem_CreateDir callback) {
        TF_FilesystemOps_SetCreateDir(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_recursively_create_dir(TP_Filesystem_RecursivelyCreateDir callback) {
        TF_FilesystemOps_SetRecursivelyCreateDir(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_delete_file(TP_Filesystem_DeleteFile callback) {
        TF_FilesystemOps_SetDeleteFile(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_delete_dir(TP_Filesystem_DeleteDir callback) {
        TF_FilesystemOps_SetDeleteDir(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_delete_recursively(TP_Filesystem_DeleteRecursively callback) {
        TF_FilesystemOps_SetDeleteRecursively(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_rename_file(TP_Filesystem_RenameFile callback) {
        TF_FilesystemOps_SetRenameFile(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_copy_file(TP_Filesystem_CopyFile callback) {
        TF_FilesystemOps_SetCopyFile(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_path_exists(TP_Filesystem_PathExists callback) {
        TF_FilesystemOps_SetPathExists(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_paths_exist(TP_Filesystem_PathsExist callback) {
        TF_FilesystemOps_SetPathsExist(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_stat(TP_Filesystem_Stat callback) {
        TF_FilesystemOps_SetStat(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_is_directory(TP_Filesystem_IsDirectory callback) {
        TF_FilesystemOps_SetIsDirectory(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_get_file_size(TP_Filesystem_GetFileSize callback) {
        TF_FilesystemOps_SetGetFileSize(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_translate_name(TP_Filesystem_TranslateName callback) {
        TF_FilesystemOps_SetTranslateName(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_get_children(TP_Filesystem_GetChildren callback) {
        TF_FilesystemOps_SetGetChildren(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_get_matching_paths(TP_Filesystem_GetMatchingPaths callback) {
        TF_FilesystemOps_SetGetMatchingPaths(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_flush_caches(TP_Filesystem_FlushCaches callback) {
        TF_FilesystemOps_SetFlushCaches(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_get_configuration(TP_Filesystem_GetConfiguration callback) {
        TF_FilesystemOps_SetGetFilesystemConfiguration(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_set_configuration(TP_Filesystem_SetConfiguration callback) {
        TF_FilesystemOps_SetSetFilesystemConfiguration(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_get_configuration_option(TP_Filesystem_GetConfigurationOption callback) {
        TF_FilesystemOps_SetGetFilesystemConfigurationOption(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_set_configuration_option(TP_Filesystem_SetConfigurationOption callback) {
        TF_FilesystemOps_SetSetFilesystemConfigurationOption(m_handle, callback);
        return *this;
    }
    FilesystemOpsBuilder &set_get_configuration_keys(TP_Filesystem_GetConfigurationKeys callback) {
        TF_FilesystemOps_SetGetFilesystemConfigurationKeys(m_handle, callback);
        return *this;
    }

    // Underlying handle — pass directly to the C ABI
    TF_FilesystemOps *get_handle() { return m_handle; }
    const TF_FilesystemOps *get_handle() const { return m_handle; }

  private:
    TF_FilesystemOps *m_handle;
};

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

// FilesystemPluginBuilder wrapper (owned pointer)
class FilesystemPluginBuilder {
  public:
    FilesystemPluginBuilder() : m_handle{TF_FilesystemPluginInfoNew()} {}
    ~FilesystemPluginBuilder() { TF_FilesystemPluginInfoDelete(m_handle); }

    FilesystemPluginBuilder(const FilesystemPluginBuilder &) = delete;
    FilesystemPluginBuilder &operator=(const FilesystemPluginBuilder &) = delete;

    FilesystemPluginBuilder(FilesystemPluginBuilder &&other) noexcept : m_handle{other.m_handle} {
        other.m_handle = nullptr;
    }
    FilesystemPluginBuilder &operator=(FilesystemPluginBuilder &&other) noexcept {
        if (this != &other) {
            TF_FilesystemPluginInfoDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    FilesystemPluginBuilder &set_num_schemes(size_t num) {
        TF_FilesystemPluginInfo_SetNumSchemes(m_handle, num);
        return *this;
    }
    FilesystemPluginBuilder &set_ops(TF_FilesystemPluginOps *ops) {
        TF_FilesystemPluginInfo_SetOps(m_handle, ops);
        return *this;
    }
    FilesystemPluginBuilder &set_memory_allocator(TP_FilesystemPlugin_MemoryAllocate alloc,
                                           TP_FilesystemPlugin_MemoryFree dealloc) {
        TF_FilesystemPluginInfo_SetMemoryAllocator(m_handle, alloc, dealloc);
        return *this;
    }

    // Underlying handle — pass directly to the C ABI
    TF_FilesystemPluginInfo *get_handle() { return m_handle; }
    const TF_FilesystemPluginInfo *get_handle() const { return m_handle; }

  private:
    TF_FilesystemPluginInfo *m_handle;
};

} // namespace ice
