module;

#include "c/intern/tf_buffer.h"

export module cc_abi_builder_intern:buffer;

import std;

export namespace ice {

class BufferBuilder {
  public:
    BufferBuilder() : m_buffer{TF_NewBuffer()} {}

    ~BufferBuilder() {

        if (m_buffer)
            TF_DeleteBuffer(m_buffer);

    }

    BufferBuilder(const BufferBuilder &) = delete;
    BufferBuilder &operator=(const BufferBuilder &) = delete;

    BufferBuilder(BufferBuilder &&other) noexcept : m_buffer{other.m_buffer} { other.m_buffer = nullptr; }

    BufferBuilder &operator=(BufferBuilder &&other) noexcept {

        if (this != &other) {
            if (m_buffer)
                TF_DeleteBuffer(m_buffer);
            m_buffer = other.m_buffer;
            other.m_buffer = nullptr;
        }
        return *this;

    }

    bool is_valid() const { return m_buffer != nullptr; }
    const void *get_data() const { return m_buffer->data; }
    size_t get_length() const { return m_buffer->length; }

    std::string to_string() const {

        return std::string(reinterpret_cast<const char *>(m_buffer->data), m_buffer->length);

    }

    // Underlying handle — pass directly to the C ABI
    TF_Buffer *get_handle() { return m_buffer; }
    const TF_Buffer *get_handle() const { return m_buffer; }

  private:
    TF_Buffer *m_buffer;
};

} // namespace ice
