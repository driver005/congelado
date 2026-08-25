module;

#include "c/extern/env/filesystem.h"

export module cc_abi_builder_env:string_stream_view;

export namespace ice {

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

}  // namespace ice
