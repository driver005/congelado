#include "include/c/extern/filesystem/filesystem.h"
#include "include/c/extern/filesystem/random_access_file.h"
#include "include/c/extern/filesystem/read_only_memory_region.h"
#include "include/c/extern/filesystem/tree.h"
#include "include/c/extern/filesystem/writable_file.h"
#include "include/c/extern/gradients/gradient.h"
#include "include/c/extern/grappler/configs.h"
#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/grappler/grappler.h"
#include "include/c/extern/grappler/optimizer.h"
#include "include/c/extern/plugin/registration.h"
#include "include/c/extern/profiler/profiler.h"
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
#include "include/c/intern/array.h"
#include "include/c/intern/attrtype.h"
#include "include/c/intern/bitset.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/complex.h"
#include "include/c/intern/datatype.h"
#include "include/c/intern/deque.h"
#include "include/c/intern/duration.h"
#include "include/c/intern/file_statistics.h"
#include "include/c/intern/forward_list/forward_list.h"
#include "include/c/intern/hash.h"
#include "include/c/intern/hive/hive.h"
#include "include/c/intern/list/list.h"
#include "include/c/intern/map.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/set.h"
#include "include/c/intern/shape.h"
#include "include/c/intern/span.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/time_point.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

import std;
import aten_xpu;

#define ATEN_XPU_EXPORT_OPS(name, ops_type, accessor)                                              \
    TF_CAPI_EXPORT void create_##name(                                                             \
        ops_type** ops,                                                                            \
        void** plugin_context,                                                                     \
        TF_Status* out_status                                                                      \
    )                                                                                              \
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

// Status, String and Buffer are the host's primitives: weak, so a real host definition wins, and no
// plugin initialisation (the plugin itself calls these while loading the host ops).
#define ATEN_XPU_EXPORT_WEAK_OPS(name, ops_type, accessor)                                         \
    __attribute__((weak)) TF_CAPI_EXPORT void create_##name(                                       \
        ops_type** ops,                                                                            \
        void** plugin_context,                                                                     \
        TF_Status* out_status                                                                      \
    )                                                                                              \
    {                                                                                              \
        static_cast<void>(out_status);                                                             \
        auto& registry = aten_xpu::SyclPluginRegistry::getInstance();                              \
        *ops = const_cast<ops_type*>(accessor);                                                    \
        *plugin_context = &registry;                                                               \
    }                                                                                              \
    __attribute__((weak)) TF_CAPI_EXPORT void destroy_##name(void* plugin_context)                 \
    {                                                                                              \
        static_cast<void>(plugin_context);                                                         \
    }

extern "C"
{
    TF_CAPI_EXPORT void create_plugin(TF_PluginInfo* plugin_info)
    {
        auto& registry = aten_xpu::SyclPluginRegistry::getInstance();
        registry.initialize();
        plugin_info->name = aten_xpu::k_plugin_name.data();
        plugin_info->version = aten_xpu::k_plugin_version.data();
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
    ATEN_XPU_EXPORT_OPS(
        random_generator,
        TF_RandomGeneratorOps,
        registry.getOps().getRandomGeneratorOps()
    )
    ATEN_XPU_EXPORT_OPS(tensor, TF_TensorOps, registry.getOps().getTensorOps())
    ATEN_XPU_EXPORT_OPS(
        grappler_device_graph,
        TFGrapplerDeviceGraphOps,
        registry.getOps().getDeviceGraphOps()
    )
    ATEN_XPU_EXPORT_OPS(gradient, TFGradientOps, registry.getGradientOps())
    ATEN_XPU_EXPORT_OPS(grappler, TF_GrapplerOps, &registry.getGrapplerOps())
    ATEN_XPU_EXPORT_OPS(grappler_optimizer, TFGrapplerOptimizerOps, &registry.getOptimizerOps())
    ATEN_XPU_EXPORT_OPS(grappler_configs, TFGrapplerConfigsOps, &registry.getConfigsOps())
    ATEN_XPU_EXPORT_OPS(profiler, TF_ProfilerOps, &registry.getProfilerOps())

    ATEN_XPU_EXPORT_WEAK_OPS(string, TF_StringOps, &registry.getStringOps())
    ATEN_XPU_EXPORT_WEAK_OPS(status, TF_StatusOps, &registry.getStatusOps())
    ATEN_XPU_EXPORT_WEAK_OPS(buffer, TF_BufferOps, &registry.getBufferOps())

    ATEN_XPU_EXPORT_OPS(registration, TF_RegistrationOps, &registry.getRegistrationOps())
    ATEN_XPU_EXPORT_OPS(complex, TF_ComplexOps, &registry.getComplexOps())
    ATEN_XPU_EXPORT_OPS(duration, TF_DurationOps, &registry.getDurationOps())
    ATEN_XPU_EXPORT_OPS(time_point, TF_TimePointOps, &registry.getTimePointOps())
    ATEN_XPU_EXPORT_OPS(datatype, TF_DataTypeOps, &registry.getDataTypeOps())
    ATEN_XPU_EXPORT_OPS(attrtype, TF_AttrTypeOps, &registry.getAttrTypeOps())
    ATEN_XPU_EXPORT_OPS(shape, TF_ShapeOps, &registry.getShapeOps())
    ATEN_XPU_EXPORT_OPS(file_statistics, TF_FileStatisticsOps, &registry.getFileStatisticsOps())
    ATEN_XPU_EXPORT_OPS(hash, TF_HashOps, &registry.getHashOps())
    ATEN_XPU_EXPORT_OPS(array, TF_ArrayOps, &registry.getArrayOps())
    ATEN_XPU_EXPORT_OPS(vector, TF_VectorOps, &registry.getVectorOps())
    ATEN_XPU_EXPORT_OPS(deque, TF_DequeOps, &registry.getDequeOps())
    ATEN_XPU_EXPORT_OPS(list, TF_ListOps, &registry.getListOps())
    ATEN_XPU_EXPORT_OPS(forward_list, TF_ForwardListOps, &registry.getForwardListOps())
    ATEN_XPU_EXPORT_OPS(hive, TF_HiveOps, &registry.getHiveOps())
    ATEN_XPU_EXPORT_OPS(bitset, TF_BitSetOps, &registry.getBitSetOps())
    ATEN_XPU_EXPORT_OPS(span, TF_SpanOps, &registry.getSpanOps())
    ATEN_XPU_EXPORT_OPS(map, TF_MapOps, &registry.getMapOps())
    ATEN_XPU_EXPORT_OPS(set, TF_SetOps, &registry.getSetOps())
    ATEN_XPU_EXPORT_OPS(filesystem, TF_FilesystemOps, &registry.getFilesystemOps())
    ATEN_XPU_EXPORT_OPS(
        random_access_file,
        TF_RandomAccessFileOps,
        &registry.getRandomAccessFileOps()
    )
    ATEN_XPU_EXPORT_OPS(writable_file, TF_WritableFileOps, &registry.getWritableFileOps())
    ATEN_XPU_EXPORT_OPS(
        read_only_memory_region,
        TF_ReadOnlyMemoryRegionOps,
        &registry.getReadOnlyMemoryRegionOps()
    )
    ATEN_XPU_EXPORT_OPS(filesystem_tree, TFFilesystemTreeOps, &registry.getFilesystemTreeOps())
}
