module;

#include "include/c/extern/grappler/grappler.h"
#include "include/c/extern/grappler/item.h"
#include "include/c/extern/grappler/optimizer.h"
#include "include/c/extern/kernel/construction.h"
#include "include/c/extern/kernel/context.h"
#include "include/c/extern/stream_executor/memory.h"
#include "include/c/extern/stream_executor/platform.h"
#include "include/c/extern/stream_executor/stream_executor.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/datatype.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_extern_registration:registration;

import std;
import aten_xpu_intern;
import aten_xpu_extern_stream_executor;
import aten_xpu_extern_random_generator;
import aten_xpu_extern_grappler;
import aten_xpu_extern_kernel;

export namespace aten_xpu {

inline constexpr std::string_view k_plugin_name = "aten_xpu";
inline constexpr std::string_view k_plugin_version = "0.1.0";

class SyclPluginRegistry
{
public:
    SyclPluginRegistry(const SyclPluginRegistry&) = delete;
    SyclPluginRegistry& operator=(const SyclPluginRegistry&) = delete;
    SyclPluginRegistry(SyclPluginRegistry&&) = delete;
    SyclPluginRegistry& operator=(SyclPluginRegistry&&) = delete;

    static SyclPluginRegistry& getInstance()
    {
        static SyclPluginRegistry registry;
        return registry;
    }

    void initialize()
    {
        if (m_initialized) {
            return;
        }
        m_initialized = true;
        load_host_ops();
        load_plugin_ops();
        register_kernels();
    }

    SyclOpsTable& getOps() noexcept
    {
        return m_ops;
    }

    const ::TF_PlatformOps& getPlatformOps() const noexcept
    {
        return m_platform.get_vtable();
    }

    const ::TF_MemoryOps& getMemoryOps() const noexcept
    {
        return m_memory.get_vtable();
    }

    const ::TF_StreamExecutorOps& getStreamExecutorOps() const noexcept
    {
        return m_stream_executor.get_vtable();
    }

    const ::TF_GrapplerOps& getGrapplerOps() const noexcept
    {
        return m_grappler.get_vtable();
    }

    const ::TFGrapplerOptimizerOps& getOptimizerOps() const noexcept
    {
        return m_optimizer.get_vtable();
    }

    std::size_t getKernelCount() const noexcept
    {
        return m_kernel_count;
    }

private:
    SyclPluginRegistry() :
        m_ops{SyclOpsTable::getInstance()},
        m_platform{m_ops},
        m_device{m_ops},
        m_executor{m_ops},
        m_stream{m_ops},
        m_event{m_ops},
        m_allocator{m_ops},
        m_mem_pool{m_ops},
        m_memory{m_ops},
        m_stream_executor{m_ops},
        m_random_generator{m_ops},
        m_tensor{m_ops},
        m_device_graph{m_ops},
        m_grappler{m_ops},
        m_optimizer{m_ops}
    {
    }

    ~SyclPluginRegistry() = default;

    void load_host_ops()
    {
        ::TF_StatusOps* status_ops = nullptr;
        ::TF_StringOps* string_ops = nullptr;
        ::TF_BufferOps* buffer_ops = nullptr;
        ::TF_OpKernelContextOps* context_ops = nullptr;
        ::TF_OpKernelConstructionOps* construction_ops = nullptr;
        ::TFGrapplerItemOps* item_ops = nullptr;
        create_status(&status_ops, &m_host_contexts[0], nullptr);
        create_string(&string_ops, &m_host_contexts[1], nullptr);
        create_buffer(&buffer_ops, &m_host_contexts[2], nullptr);
        create_op_kernel_context(&context_ops, &m_host_contexts[3], nullptr);
        create_op_kernel_construction(&construction_ops, &m_host_contexts[4], nullptr);
        create_grappler_item(&item_ops, &m_host_contexts[5], nullptr);

        m_ops.setStatusOps(status_ops);
        m_ops.setStringOps(string_ops);
        m_ops.setBufferOps(buffer_ops);
        m_ops.setKernelContextOps(context_ops);
        m_ops.setKernelConstructionOps(construction_ops);
        m_ops.setGrapplerItemOps(item_ops);
    }

    void load_plugin_ops()
    {
        m_platform.get_generic_vtable(
            [](::TF_Platform* handle)
            {
                auto* object = new SyclPlatform{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_device.get_generic_vtable(
            [](::TF_Device* handle)
            {
                auto* object = new SyclDevice{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_executor.get_generic_vtable(
            [](::TF_Executor* handle)
            {
                auto* object = new SyclExecutor{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_stream.get_generic_vtable(
            [](::TF_Stream* handle)
            {
                auto* object = new SyclStream{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_event.get_generic_vtable(
            [](::TF_Event* handle)
            {
                auto* object = new SyclEvent{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_timer.get_generic_vtable(
            [](::TF_Timer* handle)
            {
                auto* object = new SyclTimer{};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_allocator.get_generic_vtable(
            [](::TF_Allocator* handle)
            {
                auto* object = new SyclAllocator{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_mem_pool.get_generic_vtable(
            [](::TF_MemPool* handle)
            {
                auto* object = new SyclMemPool{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_memory.get_generic_vtable(
            [](::TF_Memory* handle)
            {
                auto* object = new SyclMemory{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_stream_executor.get_generic_vtable(
            [](::TF_StreamExecutor* handle)
            {
                auto* object = new SyclStreamExecutor{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_random_generator.get_generic_vtable(
            [](::TF_RandomGenerator* handle)
            {
                auto* object = new SyclRandomGenerator{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_tensor.get_generic_vtable(
            [](::TF_Tensor* handle)
            {
                auto* object = new SyclTensor{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_device_graph.get_generic_vtable(
            [](::TFGrapplerDeviceGraph* handle)
            {
                auto* object = new SyclDeviceGraph{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_grappler.get_generic_vtable(
            [](::TF_Grappler* handle)
            {
                auto* object = new SyclGrappler{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );
        m_optimizer.get_generic_vtable(
            [](::TFGrapplerOptimizer* handle)
            {
                auto* object = new SyclGrapplerOptimizer{SyclOpsTable::getInstance()};
                handle->plugin_data = object->get_handle().plugin_data;
            }
        );

        m_ops.setDeviceOps(&m_device.get_vtable());
        m_ops.setExecutorOps(&m_executor.get_vtable());
        m_ops.setStreamOps(&m_stream.get_vtable());
        m_ops.setEventOps(&m_event.get_vtable());
        m_ops.setTimerOps(&m_timer.get_vtable());
        m_ops.setAllocatorOps(&m_allocator.get_vtable());
        m_ops.setMemPoolOps(&m_mem_pool.get_vtable());
        m_ops.setRandomGeneratorOps(&m_random_generator.get_vtable());
        m_ops.setTensorOps(&m_tensor.get_vtable());
        m_ops.setKernelRuntime(&m_kernel_runtime);
        m_ops.setDeviceGraphOps(&m_device_graph.get_vtable());

        if (m_platform.getDeviceCount() > 0) {
            m_ops.getPlacement()
                .setPlacement(m_platform.getContext(), m_platform.getNativeDevice(0), 0);
        }
    }

    void register_kernels()
    {
        SyclKernelRegistrar registrar{m_ops};
        for (const auto type: {TF_FLOAT, TF_HALF, TF_BFLOAT16}) {
            registrar.add<SyclAddmmKernel>(k_addmm_kernel_name, type);
            registrar.add<SyclBmmKernel>(k_bmm_kernel_name, type);
            registrar.add<SyclBaddbmmKernel>(k_baddbmm_kernel_name, type);
            registrar.add<SyclAddmvKernel>(k_addmv_kernel_name, type);
            registrar.add<SyclLinearKernel>(k_linear_kernel_name, type);
            registrar.add<SyclConvolutionKernel>(k_convolution_kernel_name, type);
            registrar.add<SyclConvolutionBackwardKernel>(k_convolution_backward_kernel_name, type);
            registrar.add<SyclDeconvolutionKernel>(k_deconvolution_kernel_name, type);
            registrar.add<SyclWeightOnlyMatmulKernel>(k_weight_only_matmul_kernel_name, type);
            registrar.add<SyclSdpaKernel>(k_sdpa_kernel_name, type);
            registrar.add<SyclSdpaBackwardKernel>(k_sdpa_backward_kernel_name, type);
            registrar.add<SyclLstmKernel>(k_lstm_kernel_name, type);
        }
        for (const auto type: {TF_QINT8, TF_QUINT8}) {
            registrar.add<SyclInt8MatmulKernel>(k_int8_matmul_kernel_name, type);
            registrar.add<SyclInt8MatmulKernel>("QuantizedLinear", type);
            registrar.add<SyclInt8ConvolutionKernel>(k_int8_convolution_kernel_name, type);
        }
        for (const auto type: {TF_FLOAT8_E4M3FN, TF_FLOAT8_E5M2}) {
            registrar.add<SyclScaledMatmulKernel>(k_scaled_matmul_kernel_name, type);
        }
        m_kernel_count = registrar.getRegisteredCount();
    }

    SyclOpsTable& m_ops;
    SyclKernelRuntime m_kernel_runtime;
    std::array<void*, 6> m_host_contexts{};
    bool m_initialized{false};
    std::size_t m_kernel_count{0};
    SyclPlatform m_platform;
    SyclDevice m_device;
    SyclExecutor m_executor;
    SyclStream m_stream;
    SyclEvent m_event;
    SyclTimer m_timer;
    SyclAllocator m_allocator;
    SyclMemPool m_mem_pool;
    SyclMemory m_memory;
    SyclStreamExecutor m_stream_executor;
    SyclRandomGenerator m_random_generator;
    SyclTensor m_tensor;
    SyclDeviceGraph m_device_graph;
    SyclGrappler m_grappler;
    SyclGrapplerOptimizer m_optimizer;
};

} // namespace aten_xpu
