module;

#include "include/c/intern/buffer.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:buffer;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;

export namespace aten_xpu {

// A buffer that owns a copy of its bytes; get_buffer hands out a non-owning view of them.
class Buffer : public ice::builder::Buffer
{
public:
    explicit Buffer(const ::TF_StringOps* string_ops) noexcept :
        ice::builder::Buffer{string_ops}
    {
    }

    ~Buffer() override = default;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&&) = delete;
    Buffer& operator=(Buffer&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        out_name.copy("Buffer");
    }

    void assign_from_string(const void* proto, std::size_t proto_len) noexcept override
    {
        const auto* first = static_cast<const std::byte*>(proto);
        m_bytes.assign(first, first + proto_len);
    }

    void delete_buffer() noexcept override
    {
        delete this;
    }

    void get_buffer(::TFBufferData* out_buffer) noexcept override
    {
        out_buffer->data = m_bytes.data();
        out_buffer->length = m_bytes.size();
        out_buffer->data_deallocator = nullptr;
    }

private:
    std::vector<std::byte> m_bytes;
};

} // namespace aten_xpu
