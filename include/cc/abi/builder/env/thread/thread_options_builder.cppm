module;

#include "c/extern/env/thread.h"

export module cc_abi_builder_env:thread_options_builder;

export namespace ice {

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

inline void default_thread_options(ThreadOptionsBuilder* options) {

  TF_DefaultThreadOptions(options->get_handle());

}

}  // namespace ice
