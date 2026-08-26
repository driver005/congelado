module;

#include "c/extern/filesystem.h"

export module cc_abi_sonic_filesystem:random_access_file_runtime;

export namespace ice::sonic {

// Non-owning wrapper around a `TF_RandomAccessFile*` per-open-file handle received from a
// plugin's TP_Filesystem_NewRandomAccessFile callback.
class RandomAccessFileRuntime
{
public:
    RandomAccessFileRuntime() :
        m_handle{nullptr}
    {
    }

    explicit RandomAccessFileRuntime(TF_RandomAccessFile* handle) :
        m_handle{handle}
    {
    }

    void* get_plugin_file() const
    {
        return m_handle ? m_handle->plugin_file : nullptr;
    }

    // Underlying handle — pass directly to the C ABI
    TF_RandomAccessFile* get_handle() const
    {
        return m_handle;
    }

private:
    TF_RandomAccessFile* m_handle;
};

} // namespace ice::sonic
