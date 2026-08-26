module;

#include "c/extern/filesystem.h"

export module cc_abi_sonic_filesystem:random_access_file_ops_runtime;

export namespace ice::sonic {

// Non-owning wrapper around the `TF_RandomAccessFileOps*` vtable a plugin registered — unlike
// the other capabilities, these callbacks take the file handle directly rather than an `ext`
// user_data field, so each invoke method takes it explicitly.
class RandomAccessFileOpsRuntime
{
public:
    RandomAccessFileOpsRuntime() :
        m_handle{nullptr}
    {
    }

    explicit RandomAccessFileOpsRuntime(TF_RandomAccessFileOps* handle) :
        m_handle{handle}
    {
    }

    void invoke_cleanup(TF_RandomAccessFile* file) const
    {

        if (m_handle && m_handle->cleanup) {
            m_handle->cleanup(file);
        }
    }

    int64_t invoke_read(
        const TF_RandomAccessFile* file, uint64_t offset, size_t n, char* buffer, TF_Status* status
    ) const
    {

        return (m_handle && m_handle->read) ? m_handle->read(file, offset, n, buffer, status) : -1;
    }

    // Underlying handle — pass directly to the C ABI
    TF_RandomAccessFileOps* get_handle() const
    {
        return m_handle;
    }

private:
    TF_RandomAccessFileOps* m_handle;
};

} // namespace ice::sonic
