module;

#include "c/extern/env/filesystem.h"
#include "c/intern/tf_file_statistics.h"

export module cc_abi_sonic_env:file_system_runtime;

import :writable_file_handle_view_runtime;
import :string_stream_view_runtime;

export namespace ice::sonic {

class FileSystemRuntime
{
public:
    static void create_dir(const char* dirname, TF_Status* status)
    {

        TF_CreateDir(dirname, status);
    }

    static void delete_dir(const char* dirname, TF_Status* status)
    {

        TF_DeleteDir(dirname, status);
    }

    static void delete_recursively(
        const char* dirname, int64_t* undeleted_files, int64_t* undeleted_dirs, TF_Status* status
    )
    {

        TF_DeleteRecursively(dirname, undeleted_files, undeleted_dirs, status);
    }

    static void file_stat(const char* filename, TF_FileStatistics* stats, TF_Status* status)
    {

        TF_FileStat(filename, stats, status);
    }

    static void new_writable_file(
        const char* filename, WritableFileHandleViewRuntime* handle, TF_Status* status
    )
    {

        TF_WritableFileHandle* raw = nullptr;
        TF_NewWritableFile(filename, &raw, status);
        if (raw) {
            *handle = WritableFileHandleViewRuntime(raw);
        }
    }

    static void close_writable_file(WritableFileHandleViewRuntime* handle, TF_Status* status)
    {

        TF_CloseWritableFile(handle->get_handle(), status);
    }

    static void sync_writable_file(WritableFileHandleViewRuntime* handle, TF_Status* status)
    {

        TF_SyncWritableFile(handle->get_handle(), status);
    }

    static void flush_writable_file(WritableFileHandleViewRuntime* handle, TF_Status* status)
    {

        TF_FlushWritableFile(handle->get_handle(), status);
    }

    static void append_writable_file(
        WritableFileHandleViewRuntime* handle, const char* data, size_t length, TF_Status* status
    )
    {

        TF_AppendWritableFile(handle->get_handle(), data, length, status);
    }

    static void delete_file(const char* filename, TF_Status* status)
    {

        TF_DeleteFile(filename, status);
    }

    static StringStreamViewRuntime get_children(const char* filename, TF_Status* status)
    {

        return StringStreamViewRuntime(TF_GetChildren(filename, status));
    }

    static StringStreamViewRuntime get_local_temp_directories()
    {

        return StringStreamViewRuntime(TF_GetLocalTempDirectories());
    }

    static char* get_temp_file_name(const char* extension)
    {

        return TF_GetTempFileName(extension);
    }
};

} // namespace ice::sonic
