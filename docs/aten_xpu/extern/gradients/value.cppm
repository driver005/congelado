module;

#include "include/c/intern/tensor.h"

export module aten_xpu_extern_gradients:value;

import std;
import cc_ice_intern_sonic;

export namespace aten_xpu {

// A symbolic tensor of the graph the host is differentiating; it is released with delete_tensor.
class SyclGradientValue
{
public:
    SyclGradientValue() noexcept = default;

    SyclGradientValue(::TF_Tensor* handle, const ::TF_TensorOps* ops) noexcept :
        m_handle{handle},
        m_ops{ops}
    {
    }

    ~SyclGradientValue()
    {
        reset();
    }

    SyclGradientValue(const SyclGradientValue&) = delete;
    SyclGradientValue& operator=(const SyclGradientValue&) = delete;

    SyclGradientValue(SyclGradientValue&& other) noexcept :
        m_handle{std::exchange(other.m_handle, nullptr)},
        m_ops{other.m_ops}
    {
    }

    SyclGradientValue& operator=(SyclGradientValue&& other) noexcept
    {
        if (this != &other) {
            reset();
            m_handle = std::exchange(other.m_handle, nullptr);
            m_ops = other.m_ops;
        }

        return *this;
    }

    explicit operator bool() const noexcept
    {
        return m_handle != nullptr;
    }

    // The tensor in the form the generated gradient context takes it.
    ice::sonic::Tensor wrap() const noexcept
    {
        const ::TF_Tensor handle = m_handle == nullptr ? ::TF_Tensor{} : *m_handle;

        return ice::sonic::Tensor{m_ops, &handle};
    }

    void reset() noexcept
    {
        if (m_handle != nullptr && m_ops != nullptr) {
            m_ops->delete_tensor(m_handle);
        }
        m_handle = nullptr;
    }

private:
    ::TF_Tensor* m_handle{nullptr};
    const ::TF_TensorOps* m_ops{nullptr};
};

} // namespace aten_xpu
