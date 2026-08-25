module;

#include "c/extern/env/filesystem.h"
#include "c/intern/tf_file_statistics.h"

export module cc_abi_runtime_env:filesystem;


export namespace ice {

class WritableFileHandleViewRuntime {
 public:
  WritableFileHandleViewRuntime() : m_handle(nullptr) {}
  explicit WritableFileHandleViewRuntime(TF_WritableFileHandle* handle) : m_handle(handle) {}
  // Borrowed wrapper: the C library owns the handle, released via
  // TF_CloseWritableFile (see FileSystemRuntime::close_writable_file).
  ~WritableFileHandleViewRuntime() = default;

  WritableFileHandleViewRuntime(const WritableFileHandleViewRuntime&) = delete;
  WritableFileHandleViewRuntime& operator=(const WritableFileHandleViewRuntime&) = delete;

  WritableFileHandleViewRuntime(WritableFileHandleViewRuntime&& other) noexcept : m_handle(other.m_handle) { other.m_handle = nullptr; }
  WritableFileHandleViewRuntime& operator=(WritableFileHandleViewRuntime&& other) noexcept {
    if (this != &other) {
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  bool is_valid() const { return m_handle != nullptr; }

  // Underlying handle — pass directly to the C ABI
  TF_WritableFileHandle *get_handle() { return m_handle; }
  const TF_WritableFileHandle *get_handle() const { return m_handle; }

 private:
  TF_WritableFileHandle* m_handle;
};

class StringStreamViewRuntime {
 public:
  StringStreamViewRuntime() : m_handle(nullptr) {}
  explicit StringStreamViewRuntime(TF_StringStream* handle) : m_handle(handle) {}
  // Borrowed wrapper: release via done() (TF_StringStreamDone).
  ~StringStreamViewRuntime() = default;

  StringStreamViewRuntime(const StringStreamViewRuntime&) = delete;
  StringStreamViewRuntime& operator=(const StringStreamViewRuntime&) = delete;

  StringStreamViewRuntime(StringStreamViewRuntime&& other) noexcept : m_handle(other.m_handle) { other.m_handle = nullptr; }
  StringStreamViewRuntime& operator=(StringStreamViewRuntime&& other) noexcept {
    if (this != &other) {
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  bool is_valid() const { return m_handle != nullptr; }
  bool next(const char** result) { return TF_StringStreamNext(m_handle, result); }
  void done() { TF_StringStreamDone(m_handle); }

  // Underlying handle — pass directly to the C ABI
  TF_StringStream *get_handle() { return m_handle; }
  const TF_StringStream *get_handle() const { return m_handle; }

 private:
  TF_StringStream* m_handle;
};

class FileSystemRuntime {
 public:
  static void create_dir(const char* dirname, TF_Status* status) {
    TF_CreateDir(dirname, status);
  }

  static void delete_dir(const char* dirname, TF_Status* status) {
    TF_DeleteDir(dirname, status);
  }

  static void delete_recursively(const char* dirname, int64_t* undeleted_files,
                                 int64_t* undeleted_dirs, TF_Status* status) {
    TF_DeleteRecursively(dirname, undeleted_files, undeleted_dirs, status);
  }

  static void file_stat(const char* filename, TF_FileStatistics* stats, TF_Status* status) {
    TF_FileStat(filename, stats, status);
  }

  static void new_writable_file(const char* filename, WritableFileHandleViewRuntime* handle, TF_Status* status) {
    TF_WritableFileHandle* raw = nullptr;
    TF_NewWritableFile(filename, &raw, status);
    if (raw) *handle = WritableFileHandleViewRuntime(raw);
  }

  static void close_writable_file(WritableFileHandleViewRuntime* handle, TF_Status* status) {
    TF_CloseWritableFile(handle->get_handle(), status);
  }

  static void sync_writable_file(WritableFileHandleViewRuntime* handle, TF_Status* status) {
    TF_SyncWritableFile(handle->get_handle(), status);
  }

  static void flush_writable_file(WritableFileHandleViewRuntime* handle, TF_Status* status) {
    TF_FlushWritableFile(handle->get_handle(), status);
  }

  static void append_writable_file(WritableFileHandleViewRuntime* handle, const char* data,
                                   size_t length, TF_Status* status) {
    TF_AppendWritableFile(handle->get_handle(), data, length, status);
  }

  static void delete_file(const char* filename, TF_Status* status) {
    TF_DeleteFile(filename, status);
  }

  static StringStreamViewRuntime get_children(const char* filename, TF_Status* status) {
    return StringStreamViewRuntime(TF_GetChildren(filename, status));
  }

  static StringStreamViewRuntime get_local_temp_directories() {
    return StringStreamViewRuntime(TF_GetLocalTempDirectories());
  }

  static char* get_temp_file_name(const char* extension) {
    return TF_GetTempFileName(extension);
  }
};

}  // namespace ice
