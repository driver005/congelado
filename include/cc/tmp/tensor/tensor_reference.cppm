module;

#include <cstddef>
#include <cstdint>

export module cc_tmp:tensor_tensor_reference;

import std;
import cc_abi;
import :tensor_tensor;

export {

namespace tensorflow {

class TensorReference {
public:
    TensorReference() : m_buf{nullptr} {}
    explicit TensorReference(const Tensor& tensor) : m_buf{tensor.buffer()} {
        if (m_buf) {
            m_buf->Ref();
        }
    }

    ~TensorReference() {
        if (m_buf) {
            m_buf->Unref();
        }
    }

    TensorReference(const TensorReference& other) : m_buf{other.m_buf} {
        if (m_buf) {
            m_buf->Ref();
        }
    }

    TensorReference& operator=(const TensorReference& other) {
        if (this != &other) {
            if (other.m_buf) other.m_buf->Ref();
            if (m_buf) m_buf->Unref();
            m_buf = other.m_buf;
        }
        return *this;
    }

    TensorReference(TensorReference&& other) noexcept : m_buf{other.m_buf} {
        other.m_buf = nullptr;
    }

    TensorReference& operator=(TensorReference&& other) noexcept {
        if (this != &other) {
            if (m_buf) m_buf->Unref();
            m_buf = other.m_buf;
            other.m_buf = nullptr;
        }
        return *this;
    }

    void Unref() {
        if (m_buf) {
            m_buf->Unref();
            m_buf = nullptr;
        }
    }

    bool SharesBufferWith(const Tensor& tensor) const {
        return m_buf != nullptr && m_buf == tensor.buffer();
    }

private:
    TensorBuffer* m_buf;
};

} // namespace tensorflow

} // export
