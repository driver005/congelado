module;

#include "c/extern/filesystem.h"

export module cc_abi_builder_filesystem:random_access_file_ops_builder;

export namespace ice {

class RandomAccessFileOpsBuilder {
  public:
    RandomAccessFileOpsBuilder() : m_handle{TF_RandomAccessFileOpsNew()} {}
    ~RandomAccessFileOpsBuilder() { TF_RandomAccessFileOpsDelete(m_handle); }

    RandomAccessFileOpsBuilder(const RandomAccessFileOpsBuilder &) = delete;
    RandomAccessFileOpsBuilder &operator=(const RandomAccessFileOpsBuilder &) = delete;

    RandomAccessFileOpsBuilder(RandomAccessFileOpsBuilder &&other) noexcept : m_handle(other.m_handle) {

        other.m_handle = nullptr;

    }

    RandomAccessFileOpsBuilder &operator=(RandomAccessFileOpsBuilder &&other) noexcept {

        if (this != &other) {
            TF_RandomAccessFileOpsDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;

    }

    RandomAccessFileOpsBuilder &set_cleanup(TP_RandomAccessFile_Cleanup callback) {

        TF_RandomAccessFileOps_SetCleanup(m_handle, callback);
        return *this;

    }
    RandomAccessFileOpsBuilder &set_read(TP_RandomAccessFile_Read callback) {

        TF_RandomAccessFileOps_SetRead(m_handle, callback);
        return *this;

    }

    // Underlying handle — pass directly to the C ABI
    TF_RandomAccessFileOps *get_handle() { return m_handle; }
    const TF_RandomAccessFileOps *get_handle() const { return m_handle; }

  private:
    TF_RandomAccessFileOps *m_handle;
};

} // namespace ice
