module;

#include "c/intern/tf_tensor.h"

export module cc_abi_primitives:tensor_handle;

export namespace ice {

class TensorHandle
{
public:
    TensorHandle() = default;

    explicit TensorHandle(TF_Tensor_Handle* handle) :
        m_handle(handle)
    {
    }

    explicit TensorHandle(const TF_Tensor_Handle* handle) :
        m_handle(const_cast<TF_Tensor_Handle*>(handle))
    {
    }

    TF_Tensor_Handle* get_handle() const
    {
        return m_handle;
    }

private:
    TF_Tensor_Handle* m_handle = nullptr;
};

} // namespace ice
