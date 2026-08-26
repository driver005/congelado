module;

#include "c/extern/filesystem.h"

export module cc_abi_sonic_filesystem:writable_file_runtime;

export namespace ice::sonic {

// Non-owning wrapper around a `TF_WritableFile*` per-open-file handle.
class WritableFileRuntime
{
public:
    WritableFileRuntime() :
        m_handle{nullptr}
    {
    }

    explicit WritableFileRuntime(TF_WritableFile* handle) :
        m_handle{handle}
    {
    }

    void* get_plugin_file() const
    {
        return m_handle ? m_handle->plugin_file : nullptr;
    }

    // Underlying handle — pass directly to the C ABI
    TF_WritableFile* get_handle() const
    {
        return m_handle;
    }

private:
    TF_WritableFile* m_handle;
};

} // namespace ice::sonic
