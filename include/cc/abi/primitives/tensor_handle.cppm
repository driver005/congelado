module;

#include "c/intern/tf_tensor.h"

export module cc_abi_primitives:tensor_handle;

export namespace ice {

class TensorHandle
{
public:
    TensorHandle() noexcept = default;

    explicit TensorHandle(TF_Tensor_Handle* handle) noexcept :
        m_handle(handle)
    {
    }

    explicit TensorHandle(const TF_Tensor_Handle* handle) noexcept :
        m_handle(const_cast<TF_Tensor_Handle*>(handle))
    {
    }

    TF_Tensor_Handle* get_handle() const noexcept
    {
        return m_handle;
    }

private:
    TF_Tensor_Handle* m_handle = nullptr;
};

} // namespace ice
