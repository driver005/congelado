module;

#include "c/intern/tf_buffer.h"

export module cc_abi_sonic_intern:buffer;

import std;

export namespace ice::sonic {

// BufferRuntime — non-owning, read-only view over a `const TF_Buffer*` received from a
// plugin. No allocation, no destructor cleanup.
class BufferRuntime
{
public:
    BufferRuntime() :
        m_buffer{nullptr}
    {
    }

    explicit BufferRuntime(const TF_Buffer* buffer) :
        m_buffer{buffer}
    {
    }

    bool is_valid() const
    {
        return m_buffer != nullptr;
    }

    const void* get_data() const
    {
        return m_buffer ? m_buffer->data : nullptr;
    }

    size_t get_length() const
    {
        return m_buffer ? m_buffer->length : 0;
    }

    std::string to_string() const
    {

        if (!m_buffer) {
            return std::string();
        }
        return std::string(reinterpret_cast<const char*>(m_buffer->data), m_buffer->length);
    }

    // Underlying handle — pass directly to the C ABI
    const TF_Buffer* get_handle() const
    {
        return m_buffer;
    }

private:
    const TF_Buffer* m_buffer;
};

} // namespace ice::sonic
