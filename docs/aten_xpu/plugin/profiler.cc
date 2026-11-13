#include "include/c/extern/profiler/profiler.h"

#include "include/c/extern/plugin/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#define ATEN_XPU_DOMAIN aten_xpu::SyclPluginDomain::k_profiler
#include "docs/aten_xpu/plugin_entry.h"

import std;
import aten_xpu;

extern "C"
{
    ATEN_XPU_PLUGIN_ENTRY("aten_xpu.profiler")

    ATEN_XPU_EXPORT_OPS(profiler, TF_ProfilerOps, &registry.getProfilerOps())
}
