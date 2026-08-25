module;

#include "c/extern/env/filesystem.h"
#include "c/intern/tf_file_statistics.h"

export module cc_abi_builder_env:filesystem;

import cc_abi_builder_intern;

export namespace ice {

class WritableFileHandleView {
 public:
  WritableFileHandleView() : m_handle(nullptr) {}
  explicit WritableFileHandleView(TF_WritableFileHandle* handle) : m_handle(handle) {}
  // Borrowed wrapper: the C library owns the handle, released via
  // TF_CloseWritableFile (see FileSystem::close_writable_file).
  ~WritableFileHandleView() = default;

  WritableFileHandleView(const WritableFileHandleView&) = delete;
  WritableFileHandleView& operator=(const WritableFileHandleView&) = delete;

  WritableFileHandleView(WritableFileHandleView&& other) noexcept : m_handle(other.m_handle) { other.m_handle = nullptr; }
  WritableFileHandleView& operator=(WritableFileHandleView&& other) noexcept {
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

class StringStreamView {
 public:
  StringStreamView() : m_handle(nullptr) {}
  explicit StringStreamView(TF_StringStream* handle) : m_handle(handle) {}
  // Borrowed wrapper: release via done() (TF_StringStreamDone).
  ~StringStreamView() = default;

  StringStreamView(const StringStreamView&) = delete;
  StringStreamView& operator=(const StringStreamView&) = delete;

  StringStreamView(StringStreamView&& other) noexcept : m_handle(other.m_handle) { other.m_handle = nullptr; }
  StringStreamView& operator=(StringStreamView&& other) noexcept {
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

class FileSystem {
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

  static void new_writable_file(const char* filename, WritableFileHandleView* handle, TF_Status* status) {
    TF_WritableFileHandle* raw = nullptr;
    TF_NewWritableFile(filename, &raw, status);
    if (raw) *handle = WritableFileHandleView(raw);
  }

  static void close_writable_file(WritableFileHandleView* handle, TF_Status* status) {
    TF_CloseWritableFile(handle->get_handle(), status);
  }

  static void sync_writable_file(WritableFileHandleView* handle, TF_Status* status) {
    TF_SyncWritableFile(handle->get_handle(), status);
  }

  static void flush_writable_file(WritableFileHandleView* handle, TF_Status* status) {
    TF_FlushWritableFile(handle->get_handle(), status);
  }

  static void append_writable_file(WritableFileHandleView* handle, const char* data,
                                   size_t length, TF_Status* status) {
    TF_AppendWritableFile(handle->get_handle(), data, length, status);
  }

  static void delete_file(const char* filename, TF_Status* status) {
    TF_DeleteFile(filename, status);
  }

  static StringStreamView get_children(const char* filename, TF_Status* status) {
    return StringStreamView(TF_GetChildren(filename, status));
  }

  static StringStreamView get_local_temp_directories() {
    return StringStreamView(TF_GetLocalTempDirectories());
  }

  static char* get_temp_file_name(const char* extension) {
    return TF_GetTempFileName(extension);
  }
};

}  // namespace ice
