module;

#include "c/extern/env/thread.h"

export module cc_abi_builder_env:thread;

import cc_abi_builder_intern;


export namespace ice {

class ThreadView;
class ThreadOptionsBuilder;

inline void default_thread_options(ThreadOptionsBuilder* options);
inline ThreadView start_thread(const ThreadOptionsBuilder* options, const char* thread_name,
                           TF_ThreadWorkFn work_func, void* param);

class ThreadOptionsBuilder {
 public:
  ThreadOptionsBuilder() : m_handle{TF_ThreadOptionsNew()} {}

  ~ThreadOptionsBuilder() { TF_ThreadOptionsDelete(m_handle); }

  ThreadOptionsBuilder(const ThreadOptionsBuilder&) = delete;
  ThreadOptionsBuilder& operator=(const ThreadOptionsBuilder&) = delete;

  ThreadOptionsBuilder(ThreadOptionsBuilder&& other) noexcept : m_handle(other.m_handle) {
    other.m_handle = nullptr;
  }

  ThreadOptionsBuilder& operator=(ThreadOptionsBuilder&& other) noexcept {
    if (this != &other) {
      TF_ThreadOptionsDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  size_t get_stack_size() const { return m_handle->stack_size; }
  ThreadOptionsBuilder& set_stack_size(size_t v) {
    TF_ThreadOptions_SetStackSize(m_handle, v);
    return *this;
  }

  size_t get_guard_size() const { return m_handle->guard_size; }
  ThreadOptionsBuilder& set_guard_size(size_t v) {
    TF_ThreadOptions_SetGuardSize(m_handle, v);
    return *this;
  }

  int get_numa_node() const { return m_handle->numa_node; }
  ThreadOptionsBuilder& set_numa_node(int v) {
    TF_ThreadOptions_SetNumaNode(m_handle, v);
    return *this;
  }

  // Underlying handle — pass directly to the C ABI
  TF_ThreadOptions *get_handle() { return m_handle; }
  const TF_ThreadOptions *get_handle() const { return m_handle; }

 private:
  TF_ThreadOptions* m_handle;
};

class ThreadView {
 public:
  explicit ThreadView(TF_Thread* handle) : m_handle(handle) {}
  ~ThreadView() = default;

  ThreadView(const ThreadView&) = delete;
  ThreadView& operator=(const ThreadView&) = delete;

  ThreadView(ThreadView&& other) noexcept : m_handle(other.m_handle) { other.m_handle = nullptr; }
  ThreadView& operator=(ThreadView&& other) noexcept {
    if (this != &other) { m_handle = other.m_handle; other.m_handle = nullptr; }
    return *this;
  }

  void join() { if (m_handle) TF_JoinThread(m_handle); }

  // Underlying handle — pass directly to the C ABI
  TF_Thread *get_handle() { return m_handle; }
  const TF_Thread *get_handle() const { return m_handle; }

 private:
  TF_Thread* m_handle;
};

inline void default_thread_options(ThreadOptionsBuilder* options) {
  TF_DefaultThreadOptions(options->get_handle());
}

inline ThreadView start_thread(const ThreadOptionsBuilder* options, const char* thread_name,
                           TF_ThreadWorkFn work_func, void* param) {
  return ThreadView(TF_StartThread(const_cast<TF_ThreadOptions *>(options->get_handle()), thread_name, work_func, param));
}

}  // namespace ice
