module;

#include "c/extern/env/filesystem.h"

export module cc_abi_builder_env:writable_file_handle_view;

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

}  // namespace ice
