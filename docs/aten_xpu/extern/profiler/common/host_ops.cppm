module;

#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_profiler_common:host_ops;

import std;

export namespace aten_xpu {

// The ops tables every ice object of a profiler plugin needs: the host's status and string ops and
// the plugin's own tensor ops.
class ProfilerHostOps
{
public:
    void setStatusOps(const ::TF_StatusOps* ops) noexcept
    {
        m_status_ops = ops;
    }

    void setStringOps(const ::TF_StringOps* ops) noexcept
    {
        m_string_ops = ops;
    }

    void setTensorOps(const ::TF_TensorOps* ops) noexcept
    {
        m_tensor_ops = ops;
    }

    [[nodiscard]] const ::TF_StatusOps* getStatusOps() const noexcept
    {
        return m_status_ops;
    }

    [[nodiscard]] const ::TF_StringOps* getStringOps() const noexcept
    {
        return m_string_ops;
    }

    [[nodiscard]] const ::TF_TensorOps* getTensorOps() const noexcept
    {
        return m_tensor_ops;
    }

private:
    const ::TF_StatusOps* m_status_ops{nullptr};
    const ::TF_StringOps* m_string_ops{nullptr};
    const ::TF_TensorOps* m_tensor_ops{nullptr};
};

} // namespace aten_xpu
