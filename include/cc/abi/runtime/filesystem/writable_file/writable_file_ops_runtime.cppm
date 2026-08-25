module;

#include "c/extern/filesystem.h"

export module cc_abi_runtime_filesystem:writable_file_ops_runtime;

export namespace ice {

// Non-owning wrapper around the `TF_WritableFileOps*` vtable — callbacks take the file handle
// directly rather than an `ext` user_data field.
class WritableFileOpsRuntime {
  public:
    WritableFileOpsRuntime() : m_handle{nullptr} {}
    explicit WritableFileOpsRuntime(TF_WritableFileOps *handle) : m_handle{handle} {}

    void invoke_cleanup(TF_WritableFile *file) const {

        if (m_handle && m_handle->cleanup) {
            m_handle->cleanup(file);
        }

    }
    void invoke_append(const TF_WritableFile *file, const char *buffer, size_t n, TF_Status *status) const {

        if (m_handle && m_handle->append) {
            m_handle->append(file, buffer, n, status);
        }

    }
    int64_t invoke_tell(const TF_WritableFile *file, TF_Status *status) const {

        return (m_handle && m_handle->tell) ? m_handle->tell(file, status) : -1;

    }
    void invoke_flush(const TF_WritableFile *file, TF_Status *status) const {

        if (m_handle && m_handle->flush) {
            m_handle->flush(file, status);
        }

    }
    void invoke_sync(const TF_WritableFile *file, TF_Status *status) const {

        if (m_handle && m_handle->sync) {
            m_handle->sync(file, status);
        }

    }
    void invoke_close(const TF_WritableFile *file, TF_Status *status) const {

        if (m_handle && m_handle->close) {
            m_handle->close(file, status);
        }

    }

    // Underlying handle — pass directly to the C ABI
    TF_WritableFileOps *get_handle() const { return m_handle; }

  private:
    TF_WritableFileOps *m_handle;
};

} // namespace ice
