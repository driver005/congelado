module;

#include "c/extern/ops.h"

export module cc_abi_builder:dimension_handle;

export namespace ice {

// Owned wrapper for TF_DimensionHandle
class DimensionHandle {
 public:
  DimensionHandle() : m_handle{nullptr} {}
  explicit DimensionHandle(TF_DimensionHandle* handle) : m_handle{handle} {}
  ~DimensionHandle() { if (m_handle) TF_DeleteDimensionHandle(m_handle); }

  DimensionHandle(const DimensionHandle&) = delete;
  DimensionHandle& operator=(const DimensionHandle&) = delete;

  DimensionHandle(DimensionHandle&& other) noexcept : m_handle{other.m_handle} { other.m_handle = nullptr; }
  DimensionHandle& operator=(DimensionHandle&& other) noexcept {

    if (this != &other) {
      if (m_handle) TF_DeleteDimensionHandle(m_handle);
      m_handle = other.m_handle;
      other.m_handle = nullptr;
    }
    return *this;

  }

  static DimensionHandle create() { return DimensionHandle(TF_NewDimensionHandle()); }

  bool value_known() const { return TF_DimensionHandleValueKnown(m_handle); }
  int64_t value() const { return TF_DimensionHandleValue(m_handle); }

  // Underlying handle — pass directly to the C ABI
  TF_DimensionHandle *get_handle() { return m_handle; }
  const TF_DimensionHandle *get_handle() const { return m_handle; }

 private:
  TF_DimensionHandle* m_handle;
};

} // namespace ice
