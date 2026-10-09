module;

#include "include/c/extern/kernel/kernel.h"

export module aten_xpu_extern_kernel:kernel;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_kernel_builder;
import aten_xpu_intern;
import :context;
import :construction;

export namespace aten_xpu {

template<typename Derived>
class SyclKernel : public ice::builder::TF_KernelOps
{
public:
    explicit SyclKernel(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_KernelOps{ops.getStringOps()},
        m_status{ops}
    {
    }

    ~SyclKernel() override = default;
    SyclKernel(const SyclKernel&) = delete;
    SyclKernel& operator=(const SyclKernel&) = delete;
    SyclKernel(SyclKernel&&) = delete;
    SyclKernel& operator=(SyclKernel&&) = delete;

    static void create_kernel(::TF_OpKernelConstruction* raw_construction, void** out_plugin_data)
    {
        SyclKernelConstruction construction{raw_construction};
        auto* kernel = new Derived{SyclOpsTable::getInstance(), construction};
        if (!construction.ok()) {
            delete kernel;
            *out_plugin_data = nullptr;
            return;
        }
        *out_plugin_data = kernel;
    }

    static void compute_kernel(void* plugin_data, ::TF_OpKernelContext* raw_context)
    {
        SyclKernelContext context{raw_context};
        if (plugin_data == nullptr) {
            context.fail(TF_FAILED_PRECONDITION, "kernel construction failed");
            return;
        }
        try {
            static_cast<Derived*>(plugin_data)->compute(context);
        } catch (const std::exception& error) {
            context.fail_from(error);
        }
    }

    static void delete_kernel(void* plugin_data)
    {
        delete static_cast<Derived*>(plugin_data);
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        m_status.copy_into(out_name, Derived::k_name);
    }

private:
    SyclStatus m_status;
};

} // namespace aten_xpu
