#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/grappler/grappler.h"
#include "include/c/extern/grappler/optimizer.h"
#include "include/c/extern/plugin/registration.h"
#include "include/c/extern/random_generator/random_generator.h"
#include "include/c/extern/stream_executor/allocator.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/event.h"
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/extern/stream_executor/memory.h"
#include "include/c/extern/stream_executor/platform.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/stream_executor.h"
#include "include/c/extern/stream_executor/timer.h"
#include "include/c/intern/tensor.h"

import std;
import aten_xpu;

#define ATEN_XPU_EXPORT_OPS(name, ops_type, accessor)                                              \
    TF_CAPI_EXPORT void create_##name(ops_type** ops, void** plugin_context, TF_Status* out_status) \
    {                                                                                              \
        static_cast<void>(out_status);                                                             \
        auto& registry = aten_xpu::SyclPluginRegistry::getInstance();                              \
        registry.initialize();                                                                     \
        *ops = const_cast<ops_type*>(accessor);                                                    \
        *plugin_context = &registry;                                                               \
    }                                                                                              \
    TF_CAPI_EXPORT void destroy_##name(void* plugin_context)                                       \
    {                                                                                              \
        static_cast<void>(plugin_context);                                                         \
    }

extern "C"
{
    TF_CAPI_EXPORT void create_plugin(TF_PluginInfo* plugin_info)
    {
        auto& registry = aten_xpu::SyclPluginRegistry::getInstance();
        registry.initialize();
        plugin_info->name = aten_xpu::SyclPluginRegistry::k_name.data();
        plugin_info->version = aten_xpu::SyclPluginRegistry::k_version.data();
    }

    ATEN_XPU_EXPORT_OPS(platform, TF_PlatformOps, &registry.getPlatformOps())
    ATEN_XPU_EXPORT_OPS(device, TF_DeviceOps, registry.getOps().getDeviceOps())
    ATEN_XPU_EXPORT_OPS(executor, TF_ExecutorOps, registry.getOps().getExecutorOps())
    ATEN_XPU_EXPORT_OPS(stream, TF_StreamOps, registry.getOps().getStreamOps())
    ATEN_XPU_EXPORT_OPS(event, TF_EventOps, registry.getOps().getEventOps())
    ATEN_XPU_EXPORT_OPS(timer, TF_TimerOps, registry.getOps().getTimerOps())
    ATEN_XPU_EXPORT_OPS(allocator, TF_AllocatorOps, registry.getOps().getAllocatorOps())
    ATEN_XPU_EXPORT_OPS(mem_pool, TF_MemPoolOps, registry.getOps().getMemPoolOps())
    ATEN_XPU_EXPORT_OPS(memory, TF_MemoryOps, &registry.getMemoryOps())
    ATEN_XPU_EXPORT_OPS(stream_executor, TF_StreamExecutorOps, &registry.getStreamExecutorOps())
    ATEN_XPU_EXPORT_OPS(random_generator, TF_RandomGeneratorOps, registry.getOps().getRandomGeneratorOps())
    ATEN_XPU_EXPORT_OPS(tensor, TF_TensorOps, registry.getOps().getTensorOps())
    ATEN_XPU_EXPORT_OPS(grappler_device_graph, TFGrapplerDeviceGraphOps, registry.getOps().getDeviceGraphOps())
    ATEN_XPU_EXPORT_OPS(grappler, TF_GrapplerOps, &registry.getGrapplerOps())
    ATEN_XPU_EXPORT_OPS(grappler_optimizer, TFGrapplerOptimizerOps, &registry.getOptimizerOps())
}
