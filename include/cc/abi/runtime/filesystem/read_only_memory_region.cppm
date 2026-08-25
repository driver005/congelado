module;

#include "c/extern/filesystem.h"

export module cc_abi_runtime_filesystem:read_only_memory_region;

export namespace ice {

// Non-owning wrapper around the `TF_ReadOnlyMemoryRegionOps*` vtable — callbacks take the
// region handle directly rather than an `ext` user_data field.
class ReadOnlyMemoryRegionOpsRuntime {
  public:
    ReadOnlyMemoryRegionOpsRuntime() : m_handle{nullptr} {}
    explicit ReadOnlyMemoryRegionOpsRuntime(TF_ReadOnlyMemoryRegionOps *handle) : m_handle{handle} {}

    void invoke_cleanup(TF_ReadOnlyMemoryRegion *region) const {
        if (m_handle && m_handle->cleanup) {
            m_handle->cleanup(region);
        }
    }
    const void *invoke_data(const TF_ReadOnlyMemoryRegion *region) const {
        return (m_handle && m_handle->data) ? m_handle->data(region) : nullptr;
    }
    uint64_t invoke_length(const TF_ReadOnlyMemoryRegion *region) const {
        return (m_handle && m_handle->length) ? m_handle->length(region) : 0;
    }

    // Underlying handle — pass directly to the C ABI
    TF_ReadOnlyMemoryRegionOps *get_handle() const { return m_handle; }

  private:
    TF_ReadOnlyMemoryRegionOps *m_handle;
};

} // namespace ice
