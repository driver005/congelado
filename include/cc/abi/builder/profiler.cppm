module;

#include "c/extern/profiler.h"

export module cc_abi_builder:profiler;


export namespace ice {

// Owned pointer wrapper for TP_ProfilerFns
class ProfilerFnsBuilder {
 public:
  ProfilerFnsBuilder() : m_handle{TP_ProfilerFnsNew()} {}
  ~ProfilerFnsBuilder() { TP_ProfilerFnsDelete(m_handle); }

  ProfilerFnsBuilder(const ProfilerFnsBuilder&) = delete;
  ProfilerFnsBuilder& operator=(const ProfilerFnsBuilder&) = delete;

  ProfilerFnsBuilder(ProfilerFnsBuilder&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  ProfilerFnsBuilder& operator=(ProfilerFnsBuilder&& other) noexcept {
    if (this != &other) {
      TP_ProfilerFnsDelete(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;
  }

  ProfilerFnsBuilder& set_start(TP_ProfilerFns_Start callback) {
    TP_ProfilerFns_SetStart(m_handle, callback);
    return *this;
  }
  ProfilerFnsBuilder& set_stop(TP_ProfilerFns_Stop callback) {
    TP_ProfilerFns_SetStop(m_handle, callback);
    return *this;
  }
  ProfilerFnsBuilder& set_collect_data_xspace(TP_ProfilerFns_CollectDataXspace callback) {
    TP_ProfilerFns_SetCollectDataXspace(m_handle, callback);
    return *this;
  }

  // Underlying handle — pass directly to the C ABI
  TP_ProfilerFns *get_handle() { return m_handle; }
  const TP_ProfilerFns *get_handle() const { return m_handle; }

 private:
  TP_ProfilerFns* m_handle;
};

}  // namespace ice
