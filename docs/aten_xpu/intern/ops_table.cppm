module;

#include "include/c/extern/grappler/configs.h"
#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/grappler/function_library.h"
#include "include/c/extern/grappler/item.h"
#include "include/c/extern/grappler/properties.h"
#include "include/c/extern/gradients/context.h"
#include "include/c/extern/kernel/construction.h"
#include "include/c/extern/kernel/context.h"
#include "include/c/extern/ops/shape_handle.h"
#include "include/c/extern/ops/shape_inference_context.h"
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
import :placement;
import :graph_state;
import :shared_state;
import :profile_collector;

export namespace aten_xpu {

class SyclOpsTable
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

    void setBufferOps(const ::TF_BufferOps* ops) noexcept
    {
        m_buffer_ops = ops;
    }

    void setTensorOps(const ::TF_TensorOps* ops) noexcept
    {
        m_tensor_ops = ops;
    }

    void setDeviceOps(const ::TF_DeviceOps* ops) noexcept
    {
        m_device_ops = ops;
    }

    void setExecutorOps(const ::TF_ExecutorOps* ops) noexcept
    {
        m_executor_ops = ops;
    }

    void setStreamOps(const ::TF_StreamOps* ops) noexcept
    {
        m_stream_ops = ops;
    }

    void setEventOps(const ::TF_EventOps* ops) noexcept
    {
        m_event_ops = ops;
    }

    void setTimerOps(const ::TF_TimerOps* ops) noexcept
    {
        m_timer_ops = ops;
    }

    void setAllocatorOps(const ::TF_AllocatorOps* ops) noexcept
    {
        m_allocator_ops = ops;
    }

    void setMemPoolOps(const ::TF_MemPoolOps* ops) noexcept
    {
        m_mem_pool_ops = ops;
    }

    void setRandomGeneratorOps(const ::TF_RandomGeneratorOps* ops) noexcept
    {
        m_random_generator_ops = ops;
    }

    void setGrapplerItemOps(const ::TFGrapplerItemOps* ops) noexcept
    {
        m_grappler_item_ops = ops;
    }

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

    void setShapeInferenceContextOps(const ::TF_ShapeInferenceContextOps* ops) noexcept
    {
        m_shape_inference_context_ops = ops;
    }

    void setShapeHandleOps(const ::TF_ShapeHandleOps* ops) noexcept
    {
        m_shape_handle_ops = ops;
    }

    // What the memory and parallelism optimizers need to know about the device.
    void setDeviceInfo(
        std::uint64_t memory_bytes,
        std::uint64_t bandwidth,
        int device_count
    ) noexcept
    {
        m_device_memory_bytes = memory_bytes;
        m_device_bandwidth = bandwidth;
        m_device_count = device_count;
    }

    // The optimizer settings the host chose (set through the grappler configs object).
    void setGrapplerOptimizerConfigs(const ::TFGrapplerOptimizerConfigs& configs) noexcept
    {
        m_grappler_optimizer_configs = configs;
    }

    void setGrapplerFunctionLibraryOps(const ::TFGrapplerFunctionLibraryOps* ops) noexcept
    {
        m_grappler_function_library_ops = ops;
    }

    void setGrapplerPropertiesOps(const ::TFGrapplerPropertiesOps* ops) noexcept
    {
        m_grappler_properties_ops = ops;
    }

    void setGradientContextOps(const ::TFGradientContextOps* ops) noexcept
    {
        m_gradient_context_ops = ops;
    }

    // Entry point for the C ABI `create` slots, which carry no context pointer.
    static SyclOpsTable& getInstance() noexcept
    {
        static SyclOpsTable instance;
        return instance;
    }

    SyclTensorPlacement& getPlacement() noexcept
    {
        return m_placement;
    }

    const SyclTensorPlacement& getPlacement() const noexcept
    {
        return m_placement;
    }

    SyclGraphState& getGraphState() const noexcept
    {
        return m_graph_state;
    }

    void setKernelRuntime(SyclSharedState* runtime) noexcept
    {
        m_kernel_runtime = runtime;
    }

    SyclSharedState* getKernelRuntime() const noexcept
    {
        return m_kernel_runtime;
    }

    SyclProfileCollector& getProfileCollector() noexcept
    {
        return m_profile_collector;
    }

    const ::TF_StatusOps* getStatusOps() const noexcept
    {
        return m_status_ops;
    }

    const ::TF_StringOps* getStringOps() const noexcept
    {
        return m_string_ops;
    }

    const ::TF_BufferOps* getBufferOps() const noexcept
    {
        return m_buffer_ops;
    }

    const ::TF_TensorOps* getTensorOps() const noexcept
    {
        return m_tensor_ops;
    }

    const ::TF_DeviceOps* getDeviceOps() const noexcept
    {
        return m_device_ops;
    }

    const ::TF_ExecutorOps* getExecutorOps() const noexcept
    {
        return m_executor_ops;
    }

    const ::TF_StreamOps* getStreamOps() const noexcept
    {
        return m_stream_ops;
    }

    const ::TF_EventOps* getEventOps() const noexcept
    {
        return m_event_ops;
    }

    const ::TF_TimerOps* getTimerOps() const noexcept
    {
        return m_timer_ops;
    }

    const ::TF_AllocatorOps* getAllocatorOps() const noexcept
    {
        return m_allocator_ops;
    }

    const ::TF_MemPoolOps* getMemPoolOps() const noexcept
    {
        return m_mem_pool_ops;
    }

    const ::TF_RandomGeneratorOps* getRandomGeneratorOps() const noexcept
    {
        return m_random_generator_ops;
    }

    const ::TFGrapplerItemOps* getGrapplerItemOps() const noexcept
    {
        return m_grappler_item_ops;
    }

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

    const ::TF_ShapeInferenceContextOps* getShapeInferenceContextOps() const noexcept
    {
        return m_shape_inference_context_ops;
    }

    const ::TF_ShapeHandleOps* getShapeHandleOps() const noexcept
    {
        return m_shape_handle_ops;
    }

    std::uint64_t getDeviceMemoryBytes() const noexcept
    {
        return m_device_memory_bytes;
    }

    // Bytes per second of device memory; 0 when the device does not report it.
    std::uint64_t getDeviceBandwidth() const noexcept
    {
        return m_device_bandwidth;
    }

    int getDeviceCount() const noexcept
    {
        return m_device_count;
    }

    const ::TFGrapplerOptimizerConfigs& getGrapplerOptimizerConfigs() const noexcept
    {
        return m_grappler_optimizer_configs;
    }

    const ::TFGrapplerFunctionLibraryOps* getGrapplerFunctionLibraryOps() const noexcept
    {
        return m_grappler_function_library_ops;
    }

    const ::TFGrapplerPropertiesOps* getGrapplerPropertiesOps() const noexcept
    {
        return m_grappler_properties_ops;
    }

    const ::TFGradientContextOps* getGradientContextOps() const noexcept
    {
        return m_gradient_context_ops;
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
    const ::TF_ShapeInferenceContextOps* m_shape_inference_context_ops{nullptr};
    const ::TF_ShapeHandleOps* m_shape_handle_ops{nullptr};
    std::uint64_t m_device_memory_bytes{0};
    std::uint64_t m_device_bandwidth{0};
    int m_device_count{0};
    ::TFGrapplerOptimizerConfigs m_grappler_optimizer_configs{};
    const ::TFGrapplerFunctionLibraryOps* m_grappler_function_library_ops{nullptr};
    const ::TFGrapplerPropertiesOps* m_grappler_properties_ops{nullptr};
    const ::TFGradientContextOps* m_gradient_context_ops{nullptr};
    SyclTensorPlacement m_placement;
    mutable SyclGraphState m_graph_state;
    SyclSharedState* m_kernel_runtime{nullptr};
    SyclProfileCollector m_profile_collector;
};

} // namespace aten_xpu
