module;

#include "c/extern/filesystem.h"

export module cc_abi_builder_filesystem:filesystem_ops_builder;

export namespace ice::builder {

// FilesystemOpsBuilder wrapper (owned pointer)
class FilesystemOpsBuilder
{
public:
    FilesystemOpsBuilder() :
        m_handle{TF_FilesystemOpsNew()}
    {
    }

    ~FilesystemOpsBuilder()
    {
        TF_FilesystemOpsDelete(m_handle);
    }

    FilesystemOpsBuilder(const FilesystemOpsBuilder&) = delete;
    FilesystemOpsBuilder& operator=(const FilesystemOpsBuilder&) = delete;

    FilesystemOpsBuilder(FilesystemOpsBuilder&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    FilesystemOpsBuilder& operator=(FilesystemOpsBuilder&& other) noexcept
    {

        if (this != &other) {
            TF_FilesystemOpsDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    FilesystemOpsBuilder& set_init(TP_Filesystem_Init callback)
    {

        TF_FilesystemOps_SetInit(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_cleanup(TP_Filesystem_Cleanup callback)
    {

        TF_FilesystemOps_SetCleanup(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_new_random_access_file(TP_Filesystem_NewRandomAccessFile callback)
    {

        TF_FilesystemOps_SetNewRandomAccessFile(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_new_writable_file(TP_Filesystem_NewWritableFile callback)
    {

        TF_FilesystemOps_SetNewWritableFile(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_new_appendable_file(TP_Filesystem_NewAppendableFile callback)
    {

        TF_FilesystemOps_SetNewAppendableFile(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_new_read_only_memory_region_from_file(
        TP_Filesystem_NewReadOnlyMemoryRegionFromFile callback
    )
    {

        TF_FilesystemOps_SetNewReadOnlyMemoryRegionFromFile(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_create_dir(TP_Filesystem_CreateDir callback)
    {

        TF_FilesystemOps_SetCreateDir(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_recursively_create_dir(TP_Filesystem_RecursivelyCreateDir callback)
    {

        TF_FilesystemOps_SetRecursivelyCreateDir(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_delete_file(TP_Filesystem_DeleteFile callback)
    {

        TF_FilesystemOps_SetDeleteFile(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_delete_dir(TP_Filesystem_DeleteDir callback)
    {

        TF_FilesystemOps_SetDeleteDir(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_delete_recursively(TP_Filesystem_DeleteRecursively callback)
    {

        TF_FilesystemOps_SetDeleteRecursively(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_rename_file(TP_Filesystem_RenameFile callback)
    {

        TF_FilesystemOps_SetRenameFile(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_copy_file(TP_Filesystem_CopyFile callback)
    {

        TF_FilesystemOps_SetCopyFile(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_path_exists(TP_Filesystem_PathExists callback)
    {

        TF_FilesystemOps_SetPathExists(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_paths_exist(TP_Filesystem_PathsExist callback)
    {

        TF_FilesystemOps_SetPathsExist(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_stat(TP_Filesystem_Stat callback)
    {

        TF_FilesystemOps_SetStat(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_is_directory(TP_Filesystem_IsDirectory callback)
    {

        TF_FilesystemOps_SetIsDirectory(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_get_file_size(TP_Filesystem_GetFileSize callback)
    {

        TF_FilesystemOps_SetGetFileSize(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_translate_name(TP_Filesystem_TranslateName callback)
    {

        TF_FilesystemOps_SetTranslateName(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_get_children(TP_Filesystem_GetChildren callback)
    {

        TF_FilesystemOps_SetGetChildren(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_get_matching_paths(TP_Filesystem_GetMatchingPaths callback)
    {

        TF_FilesystemOps_SetGetMatchingPaths(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_flush_caches(TP_Filesystem_FlushCaches callback)
    {

        TF_FilesystemOps_SetFlushCaches(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_get_configuration(TP_Filesystem_GetConfiguration callback)
    {

        TF_FilesystemOps_SetGetFilesystemConfiguration(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_set_configuration(TP_Filesystem_SetConfiguration callback)
    {

        TF_FilesystemOps_SetSetFilesystemConfiguration(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder&
    set_get_configuration_option(TP_Filesystem_GetConfigurationOption callback)
    {

        TF_FilesystemOps_SetGetFilesystemConfigurationOption(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder&
    set_set_configuration_option(TP_Filesystem_SetConfigurationOption callback)
    {

        TF_FilesystemOps_SetSetFilesystemConfigurationOption(m_handle, callback);
        return *this;
    }

    FilesystemOpsBuilder& set_get_configuration_keys(TP_Filesystem_GetConfigurationKeys callback)
    {

        TF_FilesystemOps_SetGetFilesystemConfigurationKeys(m_handle, callback);
        return *this;
    }

    // Underlying handle — pass directly to the C ABI
    TF_FilesystemOps* get_handle()
    {
        return m_handle;
    }

    const TF_FilesystemOps* get_handle() const
    {
        return m_handle;
    }

private:
    TF_FilesystemOps* m_handle;
};

} // namespace ice::builder
