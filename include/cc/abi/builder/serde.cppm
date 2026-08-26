module;

#include "c/extern/serde.h"

export module cc_abi_builder:serde;

import cc_abi_sonic_intern;

export namespace ice::builder {

class SerdeBuilder
{
public:
    SerdeBuilder() :
        m_handle{TP_SerdeNew()}
    {
    }

    ~SerdeBuilder()
    {
        TP_SerdeDelete(m_handle);
    }

    SerdeBuilder(const SerdeBuilder&) = delete;
    SerdeBuilder& operator=(const SerdeBuilder&) = delete;

    SerdeBuilder(SerdeBuilder&& other) noexcept :
        m_handle{other.m_handle}
    {
        other.m_handle = nullptr;
    }

    SerdeBuilder& operator=(SerdeBuilder&& other) noexcept
    {

        if (this != &other) {
            TP_SerdeDelete(m_handle);
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    SerdeBuilder& set_encode(TP_Serde_EncodeFn callback)
    {

        TP_Serde_SetEncodeCallback(m_handle, callback);
        return *this;
    }

    SerdeBuilder& set_decode(TP_Serde_DecodeFn callback)
    {

        TP_Serde_SetDecodeCallback(m_handle, callback);
        return *this;
    }

    ice::sonic::StringRuntime get_content_type()
    {
        return ice::sonic::StringRuntime{&m_handle->content_type};
    }

    ice::sonic::StringRuntime get_format_name()
    {
        return ice::sonic::StringRuntime{&m_handle->format_name};
    }

    // Underlying handle — pass directly to the C ABI
    TP_Serde* get_handle()
    {
        return m_handle;
    }

    const TP_Serde* get_handle() const
    {
        return m_handle;
    }

private:
    TP_Serde* m_handle;
};

} // namespace ice::builder
