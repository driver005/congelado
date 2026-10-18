module;

#include "include/c/extern/kernel/builder.h"
#include "include/c/extern/kernel/construction.h"
#include "include/c/extern/kernel/context.h"

export module aten_xpu_extern_kernel:registrar;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_kernel_sonic;
import aten_xpu_intern;
import :context;
import :construction;

export namespace aten_xpu {

class SyclKernelRegistrar
{
public:
    explicit SyclKernelRegistrar(const SyclOpsTable& ops) :
        m_ops{ops},
        m_helper{ops},
        m_status{ops.getStatusOps()},
        m_scratch_first{ops.getStringOps()},
        m_scratch_second{ops.getStringOps()}
    {
        m_status.create();
        m_scratch_first.create();
        m_scratch_second.create();
    }

    ~SyclKernelRegistrar()
    {
        m_scratch_second.destroy();
        m_scratch_first.destroy();
        m_status.destroy();
    }

    SyclKernelRegistrar(const SyclKernelRegistrar&) = delete;
    SyclKernelRegistrar& operator=(const SyclKernelRegistrar&) = delete;
    SyclKernelRegistrar(SyclKernelRegistrar&&) = delete;
    SyclKernelRegistrar& operator=(SyclKernelRegistrar&&) = delete;

    template<typename Kernel>
    bool add(
        std::string_view op_name,
        std::span<const std::pair<std::string_view, ice::sonic::DataTypeEnum>> constraints
    )
    {
        ::TF_KernelBuilderOps* builder_ops = nullptr;
        ::TF_KernelBuilder handle{};
        assign(m_scratch_first, op_name);
        assign(m_scratch_second, m_device_type);
        create_kernel_builder(
            &builder_ops,
            &handle.plugin_data,
            m_scratch_first.get_handle(),
            m_scratch_second.get_handle(),
            [](::TF_OpKernelConstruction* raw_construction, void** out_plugin_data)
            {
                SyclKernelConstruction construction{raw_construction};
                auto* kernel = new Kernel{SyclOpsTable::getInstance(), construction};
                if (!construction.ok()) {
                    delete kernel;
                    *out_plugin_data = nullptr;
                    return;
                }
                *out_plugin_data = kernel;
            },
            [](void* plugin_data, ::TF_OpKernelContext* raw_context)
            {
                SyclKernelContext context{raw_context};
                if (plugin_data == nullptr) {
                    context.fail(
                        ice::sonic::Code::k_failed_precondition,
                        "kernel construction failed"
                    );
                    return;
                }
                try {
                    static_cast<Kernel*>(plugin_data)->compute(context);
                } catch (const std::exception& error) {
                    context.fail_from(error);
                }
            },
            [](void* plugin_data)
            {
                delete static_cast<Kernel*>(plugin_data);
            },
            m_status.get_handle()
        );
        if (!ok()) {
            return false;
        }

        ice::sonic::KernelBuilder builder{builder_ops, &handle};
        for (const auto& [attribute, type]: constraints) {
            assign(m_scratch_first, attribute);
            builder.type_constraint(m_scratch_first, type, m_status);
        }
        assign(m_scratch_first, op_name);
        builder.register_kernel_builder(m_scratch_first, m_status);
        destroy_kernel_builder(handle.plugin_data);
        ++m_registered;
        return ok();
    }

    template<typename Kernel>
    bool add(std::string_view op_name, ice::sonic::DataTypeEnum type)
    {
        const std::array<std::pair<std::string_view, ice::sonic::DataTypeEnum>, 1> constraints{
            {{"T", type}}
        };
        return add<Kernel>(op_name, constraints);
    }

    bool ok() const noexcept
    {
        ice::sonic::Code code{ice::sonic::Code::k_ok};
        m_status.get_code(code);
        return code == ice::sonic::Code::k_ok;
    }

    std::size_t getRegisteredCount() const noexcept
    {
        return m_registered;
    }

    const ice::sonic::Status& getStatus() const noexcept
    {
        return m_status;
    }

private:
    void assign(const ice::sonic::String& target, std::string_view value) const
    {
        target.copy(value.data(), value.size());
    }

    const SyclOpsTable& m_ops;
    std::string_view m_device_type{"XPU"};
    SyclStatus m_helper;
    ice::sonic::Status m_status;
    ice::sonic::String m_scratch_first;
    ice::sonic::String m_scratch_second;
    std::size_t m_registered{0};
};

} // namespace aten_xpu
