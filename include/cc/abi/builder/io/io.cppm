module;

#include "c/extern/io/registration.h"

export module cc_abi_builder_io:io;

export namespace ice::builder {

// Owns the TP_IO handle. The plugin sets create_request_cb/create_response_cb (via whatever
// registration path fills them — this ABI doesn't expose set_* wrappers for them the way the
// other capabilities do) and hands this off to the host. Invocation (calling those callbacks)
// is IoRuntime's job, not this class's — see runtime/io/io.cppm.
class IoBuilder
{
public:
    IoBuilder() :
        m_handle{TP_IONew()}
    {
    }

    ~IoBuilder()
    {
        TP_IODelete(m_handle);
    }

    IoBuilder(const IoBuilder&) = delete;
    IoBuilder& operator=(const IoBuilder&) = delete;

    IoBuilder(IoBuilder&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    IoBuilder& operator=(IoBuilder&& other) noexcept
    {

        if (this != &other) {
            TP_IODelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    // Underlying handle — pass directly to the C ABI
    TP_IO* get_handle()
    {
        return m_handle;
    }

    const TP_IO* get_handle() const
    {
        return m_handle;
    }

private:
    TP_IO* m_handle;
};

} // namespace ice::builder
