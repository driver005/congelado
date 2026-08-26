module;

#include "c/extern/env/filesystem.h"

export module cc_abi_sonic_env:writable_file_handle_view_runtime;

export namespace ice::sonic {

class WritableFileHandleViewRuntime
{
public:
    WritableFileHandleViewRuntime() :
        m_handle(nullptr)
    {
    }

    explicit WritableFileHandleViewRuntime(TF_WritableFileHandle* handle) :
        m_handle(handle)
    {
    }

    // Borrowed wrapper: the C library owns the handle, released via
    // TF_CloseWritableFile (see FileSystemRuntime::close_writable_file).
    ~WritableFileHandleViewRuntime() = default;

    WritableFileHandleViewRuntime(const WritableFileHandleViewRuntime&) = delete;
    WritableFileHandleViewRuntime& operator=(const WritableFileHandleViewRuntime&) = delete;

    WritableFileHandleViewRuntime(WritableFileHandleViewRuntime&& other) noexcept :
        m_handle(other.m_handle)
    {
        other.m_handle = nullptr;
    }

    WritableFileHandleViewRuntime& operator=(WritableFileHandleViewRuntime&& other) noexcept
    {

        if (this != &other) {
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    bool is_valid() const
    {
        return m_handle != nullptr;
    }

    // Underlying handle — pass directly to the C ABI
    TF_WritableFileHandle* get_handle()
    {
        return m_handle;
    }

    const TF_WritableFileHandle* get_handle() const
    {
        return m_handle;
    }

private:
    TF_WritableFileHandle* m_handle;
};

} // namespace ice::sonic
