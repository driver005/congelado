module;

#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/kernel/construction.h"
#include "include/c/extern/kernel/context.h"
#include "include/c/extern/grappler/item.h"
#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/extern/stream_executor/allocator.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/event.h"
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/timer.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:ops_table;

import std;

export namespace aten_xpu {

class SyclOpsTable
{
public:
    void setStatusOps(const ::TF_StatusOps* ops) noexcept { m_status_ops = ops; }

    void setStringOps(const ::TF_StringOps* ops) noexcept { m_string_ops = ops; }

    void setBufferOps(const ::TF_BufferOps* ops) noexcept { m_buffer_ops = ops; }

    void setTensorOps(const ::TF_TensorOps* ops) noexcept { m_tensor_ops = ops; }

    void setDeviceOps(const ::TF_DeviceOps* ops) noexcept { m_device_ops = ops; }

    void setExecutorOps(const ::TF_ExecutorOps* ops) noexcept { m_executor_ops = ops; }

    void setStreamOps(const ::TF_StreamOps* ops) noexcept { m_stream_ops = ops; }

    void setEventOps(const ::TF_EventOps* ops) noexcept { m_event_ops = ops; }

    void setTimerOps(const ::TF_TimerOps* ops) noexcept { m_timer_ops = ops; }

    void setAllocatorOps(const ::TF_AllocatorOps* ops) noexcept { m_allocator_ops = ops; }

    void setMemPoolOps(const ::TF_MemPoolOps* ops) noexcept { m_mem_pool_ops = ops; }

    void setRandomGeneratorOps(const ::TF_RandomGeneratorOps* ops) noexcept
    {

        m_random_generator_ops = ops;

    }

    void setGrapplerItemOps(const ::TFGrapplerItemOps* ops) noexcept { m_grappler_item_ops = ops; }

    void setKernelContextOps(const ::TF_OpKernelContextOps* ops) noexcept
    {

        m_kernel_context_ops = ops;

    }

    void setKernelConstructionOps(const ::TF_OpKernelConstructionOps* ops) noexcept
    {

        m_kernel_construction_ops = ops;

    }

    void setDeviceGraphOps(const ::TFGrapplerDeviceGraphOps* ops) noexcept
    {

        m_device_graph_ops = ops;

    }

    static SyclOpsTable& getInstance() noexcept
    {

        static SyclOpsTable instance;
        return instance;

    }

    const ::TF_StatusOps* getStatusOps() const noexcept { return m_status_ops; }

    const ::TF_StringOps* getStringOps() const noexcept { return m_string_ops; }

    const ::TF_BufferOps* getBufferOps() const noexcept { return m_buffer_ops; }

    const ::TF_TensorOps* getTensorOps() const noexcept { return m_tensor_ops; }

    const ::TF_DeviceOps* getDeviceOps() const noexcept { return m_device_ops; }

    const ::TF_ExecutorOps* getExecutorOps() const noexcept { return m_executor_ops; }

    const ::TF_StreamOps* getStreamOps() const noexcept { return m_stream_ops; }

    const ::TF_EventOps* getEventOps() const noexcept { return m_event_ops; }

    const ::TF_TimerOps* getTimerOps() const noexcept { return m_timer_ops; }

    const ::TF_AllocatorOps* getAllocatorOps() const noexcept { return m_allocator_ops; }

    const ::TF_MemPoolOps* getMemPoolOps() const noexcept { return m_mem_pool_ops; }

    const ::TF_RandomGeneratorOps* getRandomGeneratorOps() const noexcept
    {

        return m_random_generator_ops;

    }

    const ::TFGrapplerItemOps* getGrapplerItemOps() const noexcept { return m_grappler_item_ops; }

    const ::TF_OpKernelContextOps* getKernelContextOps() const noexcept
    {

        return m_kernel_context_ops;

    }

    const ::TF_OpKernelConstructionOps* getKernelConstructionOps() const noexcept
    {

        return m_kernel_construction_ops;

    }

    const ::TFGrapplerDeviceGraphOps* getDeviceGraphOps() const noexcept
    {

        return m_device_graph_ops;

    }

private:
    const ::TF_StatusOps* m_status_ops{nullptr};
    const ::TF_StringOps* m_string_ops{nullptr};
    const ::TF_BufferOps* m_buffer_ops{nullptr};
    const ::TF_TensorOps* m_tensor_ops{nullptr};
    const ::TF_DeviceOps* m_device_ops{nullptr};
    const ::TF_ExecutorOps* m_executor_ops{nullptr};
    const ::TF_StreamOps* m_stream_ops{nullptr};
    const ::TF_EventOps* m_event_ops{nullptr};
    const ::TF_TimerOps* m_timer_ops{nullptr};
    const ::TF_AllocatorOps* m_allocator_ops{nullptr};
    const ::TF_MemPoolOps* m_mem_pool_ops{nullptr};
    const ::TF_RandomGeneratorOps* m_random_generator_ops{nullptr};
    const ::TFGrapplerItemOps* m_grappler_item_ops{nullptr};
    const ::TFGrapplerDeviceGraphOps* m_device_graph_ops{nullptr};
    const ::TF_OpKernelContextOps* m_kernel_context_ops{nullptr};
    const ::TF_OpKernelConstructionOps* m_kernel_construction_ops{nullptr};
};

} // namespace aten_xpu
