module;

#include "c/extern/filesystem.h"

export module cc_abi_builder_filesystem:read_only_memory_region;

import cc_abi_builder_intern;

export namespace ice {

// Owned pointer wrapper for TF_ReadOnlyMemoryRegionOps
class ReadOnlyMemoryRegionOpsBuilder {
  public:
    ReadOnlyMemoryRegionOpsBuilder() : m_handle{TF_ReadOnlyMemoryRegionOpsNew()} {}
    ~ReadOnlyMemoryRegionOpsBuilder() { TF_ReadOnlyMemoryRegionOpsDelete(m_handle); }

    ReadOnlyMemoryRegionOpsBuilder(const ReadOnlyMemoryRegionOpsBuilder &) = delete;
    ReadOnlyMemoryRegionOpsBuilder &operator=(const ReadOnlyMemoryRegionOpsBuilder &) = delete;

    ReadOnlyMemoryRegionOpsBuilder(ReadOnlyMemoryRegionOpsBuilder &&other) noexcept : m_handle{other.m_handle} {
        other.m_handle = nullptr;
    }
    ReadOnlyMemoryRegionOpsBuilder &operator=(ReadOnlyMemoryRegionOpsBuilder &&other) noexcept {

        if (this != &other) {
            TF_ReadOnlyMemoryRegionOpsDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;

    }

    ReadOnlyMemoryRegionOpsBuilder &set_cleanup(TP_ReadOnlyMemoryRegion_Cleanup callback) {

        TF_ReadOnlyMemoryRegionOps_SetCleanup(m_handle, callback);
        return *this;

    }
    ReadOnlyMemoryRegionOpsBuilder &set_data(TP_ReadOnlyMemoryRegion_Data callback) {

        TF_ReadOnlyMemoryRegionOps_SetData(m_handle, callback);
        return *this;

    }
    ReadOnlyMemoryRegionOpsBuilder &set_length(TP_ReadOnlyMemoryRegion_Length callback) {

        TF_ReadOnlyMemoryRegionOps_SetLength(m_handle, callback);
        return *this;

    }

    // Underlying handle — pass directly to the C ABI
    TF_ReadOnlyMemoryRegionOps *get_handle() { return m_handle; }
    const TF_ReadOnlyMemoryRegionOps *get_handle() const { return m_handle; }

  private:
    TF_ReadOnlyMemoryRegionOps *m_handle;
};

} // namespace ice
