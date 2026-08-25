module;

#include "c/intern/tf_tensor.h"

export module cc_abi_builder_intern:tensor;

import std;
import cc_abi_value;

export namespace ice {

class TensorBuilder {
  public:
    TensorBuilder() : m_tensor{nullptr} {}
    explicit TensorBuilder(TF_Tensor *tensor) : m_tensor{tensor} {}

    ~TensorBuilder() {
        if (m_tensor)
            TF_DeleteTensor(m_tensor);
    }

    TensorBuilder(const TensorBuilder &) = delete;
    TensorBuilder &operator=(const TensorBuilder &) = delete;

    TensorBuilder(TensorBuilder &&other) noexcept : m_tensor{other.m_tensor} { other.m_tensor = nullptr; }

    TensorBuilder &operator=(TensorBuilder &&other) noexcept {
        if (this != &other) {
            if (m_tensor)
                TF_DeleteTensor(m_tensor);
            m_tensor = other.m_tensor;
            other.m_tensor = nullptr;
        }
        return *this;
    }

    bool is_valid() const { return m_tensor != nullptr; }

    DataType get_type() const { return static_cast<DataType>(TF_TensorType(m_tensor)); }

    int get_num_dims() const { return TF_NumDims(m_tensor); }
    int64_t get_dim(int index) const { return TF_Dim(m_tensor, index); }
    int64_t get_num_elements() const { return TF_TensorElementCount(m_tensor); }
    size_t get_byte_size() const { return TF_TensorByteSize(m_tensor); }
    void *get_data() const { return TF_TensorData(m_tensor); }

    TF_Tensor *release() {
        TF_Tensor *t = m_tensor;
        m_tensor = nullptr;
        return t;
    }

    // Underlying handle — pass directly to the C ABI
    TF_Tensor *get_handle() { return m_tensor; }
    const TF_Tensor *get_handle() const { return m_tensor; }

  private:
    TF_Tensor *m_tensor;
};

} // namespace ice
