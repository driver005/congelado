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

class SyclPluginRegistry
{
public:
    static constexpr std::string_view k_name = "aten_xpu";
    static constexpr std::string_view k_version = "0.1.0";

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
        m_platform.get_generic_vtable(&SyclPlatform::create);
        m_device.get_generic_vtable(&SyclDevice::create);
        m_executor.get_generic_vtable(&SyclExecutor::create);
        m_stream.get_generic_vtable(&SyclStream::create);
        m_event.get_generic_vtable(&SyclEvent::create);
        m_timer.get_generic_vtable(&SyclTimer::create);
        m_allocator.get_generic_vtable(&SyclAllocator::create);
        m_mem_pool.get_generic_vtable(&SyclMemPool::create);
        m_memory.get_generic_vtable(&SyclMemory::create);
        m_stream_executor.get_generic_vtable(&SyclStreamExecutor::create);
        m_random_generator.get_generic_vtable(&SyclRandomGenerator::create);
        m_tensor.get_generic_vtable(&SyclTensor::create);
        m_device_graph.get_generic_vtable(&SyclDeviceGraph::create);
        m_grappler.get_generic_vtable(&SyclGrappler::create);
        m_optimizer.get_generic_vtable(&SyclGrapplerOptimizer::create);

        m_ops.setDeviceOps(&m_device.get_vtable());
        m_ops.setExecutorOps(&m_executor.get_vtable());
        m_ops.setStreamOps(&m_stream.get_vtable());
        m_ops.setEventOps(&m_event.get_vtable());
        m_ops.setTimerOps(&m_timer.get_vtable());
        m_ops.setAllocatorOps(&m_allocator.get_vtable());
        m_ops.setMemPoolOps(&m_mem_pool.get_vtable());
        m_ops.setRandomGeneratorOps(&m_random_generator.get_vtable());
        m_ops.setTensorOps(&m_tensor.get_vtable());
        m_ops.setDeviceGraphOps(&m_device_graph.get_vtable());

        if (m_platform.getDeviceCount() > 0) {
            SyclTensor::setPlacement(m_platform.getContext(), m_platform.getNativeDevice(0), 0);
        }
    }

    void register_kernels()
    {
        SyclKernelRegistrar registrar{m_ops};
        for (const auto type: {TF_FLOAT, TF_HALF, TF_BFLOAT16}) {
            registrar.add<SyclAddmmKernel>(SyclAddmmKernel::k_name, type);
            registrar.add<SyclBmmKernel>(SyclBmmKernel::k_name, type);
            registrar.add<SyclBaddbmmKernel>(SyclBaddbmmKernel::k_name, type);
            registrar.add<SyclAddmvKernel>(SyclAddmvKernel::k_name, type);
            registrar.add<SyclLinearKernel>(SyclLinearKernel::k_name, type);
            registrar.add<SyclConvolutionKernel>(SyclConvolutionKernel::k_name, type);
            registrar.add<SyclConvolutionBackwardKernel>(
                SyclConvolutionBackwardKernel::k_name,
                type
            );
            registrar.add<SyclDeconvolutionKernel>(SyclDeconvolutionKernel::k_name, type);
            registrar.add<SyclWeightOnlyMatmulKernel>(SyclWeightOnlyMatmulKernel::k_name, type);
            registrar.add<SyclSdpaKernel>(SyclSdpaKernel::k_name, type);
            registrar.add<SyclSdpaBackwardKernel>(SyclSdpaBackwardKernel::k_name, type);
            registrar.add<SyclLstmKernel>(SyclLstmKernel::k_name, type);
        }
        for (const auto type: {TF_QINT8, TF_QUINT8}) {
            registrar.add<SyclInt8MatmulKernel>(SyclInt8MatmulKernel::k_name, type);
            registrar.add<SyclInt8MatmulKernel>("QuantizedLinear", type);
            registrar.add<SyclInt8ConvolutionKernel>(SyclInt8ConvolutionKernel::k_name, type);
        }
        for (const auto type: {TF_FLOAT8_E4M3FN, TF_FLOAT8_E5M2}) {
            registrar.add<SyclScaledMatmulKernel>(SyclScaledMatmulKernel::k_name, type);
        }
        m_kernel_count = registrar.getRegisteredCount();
    }

    SyclOpsTable& m_ops;
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
