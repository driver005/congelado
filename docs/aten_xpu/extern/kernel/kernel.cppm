module;

#include "include/c/extern/kernel/kernel.h"

export module aten_xpu_extern_kernel:kernel;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_kernel_builder;
import aten_xpu_intern;
import :context;
import :construction;
import :runtime;

export namespace aten_xpu {

template<typename Derived>
class SyclKernel : public ice::builder::Kernel
{
public:
    SyclKernel(const SyclOpsTable& ops, std::string_view name) noexcept :
        ice::builder::Kernel{ops.getStringOps()},
        m_status{ops},
        m_runtime{static_cast<SyclKernelRuntime&>(*ops.getKernelRuntime())},
        m_name{name}
    {
    }

    ~SyclKernel() override = default;
    SyclKernel(const SyclKernel&) = delete;
    SyclKernel& operator=(const SyclKernel&) = delete;
    SyclKernel(SyclKernel&&) = delete;
    SyclKernel& operator=(SyclKernel&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    SyclKernelRuntime& getRuntime() const noexcept
    {
        return m_runtime;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        m_status.copy_into(out_name, m_name);
    }

private:
    SyclStatus m_status;
    SyclKernelRuntime& m_runtime;
    std::string_view m_name;
};

} // namespace aten_xpu
