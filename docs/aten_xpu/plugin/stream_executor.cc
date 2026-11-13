#include "include/c/extern/stream_executor/stream_executor.h"

#include "include/c/extern/plugin/registration.h"
#include "include/c/extern/stream_executor/allocator.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/event.h"
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/extern/stream_executor/memory.h"
#include "include/c/extern/stream_executor/platform.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/timer.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#define ATEN_XPU_DOMAIN aten_xpu::SyclPluginDomain::k_stream_executor
#include "docs/aten_xpu/plugin_entry.h"

import std;
import aten_xpu;

extern "C"
{
    ATEN_XPU_PLUGIN_ENTRY("aten_xpu.stream_executor")

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
}
