module;

#include "c/intern/tf_tensor.h"

export module cc_abi_runtime_intern:tensor;

import std;
import cc_abi_value;

export namespace ice {

// TensorRuntime — non-owning, read-only view over a `TF_Tensor*` received from a plugin.
// No allocation, no `release()` (that's an ownership transfer, which doesn't apply to a
// borrowed view), no destructor cleanup.
class TensorRuntime {
  public:
    TensorRuntime() : m_tensor{nullptr} {}
    explicit TensorRuntime(TF_Tensor *tensor) : m_tensor{tensor} {}

    bool is_valid() const { return m_tensor != nullptr; }

    DataType get_type() const { return static_cast<DataType>(TF_TensorType(m_tensor)); }

    int get_num_dims() const { return TF_NumDims(m_tensor); }
    int64_t get_dim(int index) const { return TF_Dim(m_tensor, index); }
    int64_t get_num_elements() const { return TF_TensorElementCount(m_tensor); }
    size_t get_byte_size() const { return TF_TensorByteSize(m_tensor); }
    void *get_data() const { return TF_TensorData(m_tensor); }

    // Underlying handle — pass directly to the C ABI
    TF_Tensor *get_handle() const { return m_tensor; }

  private:
    TF_Tensor *m_tensor;
};

} // namespace ice
