module;

#include "c/extern/filesystem.h"

export module cc_abi_builder_filesystem:writable_file_ops_builder;

export namespace ice::builder {

class WritableFileOpsBuilder
{
public:
    WritableFileOpsBuilder() :
        m_handle{TF_WritableFileOpsNew()}
    {
    }

    ~WritableFileOpsBuilder()
    {
        TF_WritableFileOpsDelete(m_handle);
    }

    WritableFileOpsBuilder(const WritableFileOpsBuilder&) = delete;
    WritableFileOpsBuilder& operator=(const WritableFileOpsBuilder&) = delete;

    WritableFileOpsBuilder(WritableFileOpsBuilder&& other) noexcept :
        m_handle(other.m_handle)
    {

        other.m_handle = nullptr;
    }

    WritableFileOpsBuilder& operator=(WritableFileOpsBuilder&& other) noexcept
    {

        if (this != &other) {
            TF_WritableFileOpsDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    WritableFileOpsBuilder& set_cleanup(TP_WritableFile_Cleanup callback)
    {

        TF_WritableFileOps_SetCleanup(m_handle, callback);
        return *this;
    }

    WritableFileOpsBuilder& set_append(TP_WritableFile_Append callback)
    {

        TF_WritableFileOps_SetAppend(m_handle, callback);
        return *this;
    }

    WritableFileOpsBuilder& set_tell(TP_WritableFile_Tell callback)
    {

        TF_WritableFileOps_SetTell(m_handle, callback);
        return *this;
    }

    WritableFileOpsBuilder& set_flush(TP_WritableFile_Flush callback)
    {

        TF_WritableFileOps_SetFlush(m_handle, callback);
        return *this;
    }

    WritableFileOpsBuilder& set_sync(TP_WritableFile_Sync callback)
    {

        TF_WritableFileOps_SetSync(m_handle, callback);
        return *this;
    }

    WritableFileOpsBuilder& set_close(TP_WritableFile_Close callback)
    {

        TF_WritableFileOps_SetClose(m_handle, callback);
        return *this;
    }

    // Underlying handle — pass directly to the C ABI
    TF_WritableFileOps* get_handle()
    {
        return m_handle;
    }

    const TF_WritableFileOps* get_handle() const
    {
        return m_handle;
    }

private:
    TF_WritableFileOps* m_handle;
};

} // namespace ice::builder
