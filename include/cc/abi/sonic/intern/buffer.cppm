module;

#include "c/intern/tf_buffer.h"

export module cc_abi_sonic_intern:buffer;

import std;
import :runtime_base;
export namespace ice::sonic {

class Buffer : public Runtime<Buffer, TF_Buffer, true>
{
public:
    static constexpr std::string_view domain_name = "buffer";

    TF_Buffer_Handle* new_buffer_from_string(const void* proto, size_t proto_len)
    {
        return m_ops->TF_NewBufferFromString(m_host_context, proto, proto_len);
    }

    TF_Buffer_Handle* new_buffer()
    {
        return m_ops->TF_NewBuffer(m_host_context);
    }

    void delete_buffer(TF_Buffer_Handle* buffer)
    {
        m_ops->TF_DeleteBuffer(m_host_context, buffer);
    }

    TF_Buffer_Handle get_buffer(TF_Buffer_Handle* buffer)
    {
        return m_ops->TF_GetBuffer(m_host_context, buffer);
    }
};

} // namespace ice::sonic
