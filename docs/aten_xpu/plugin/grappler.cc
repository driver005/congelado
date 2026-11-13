#include "include/c/extern/grappler/grappler.h"

#include "include/c/extern/grappler/configs.h"
#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/grappler/optimizer.h"
#include "include/c/extern/plugin/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#define ATEN_XPU_DOMAIN aten_xpu::SyclPluginDomain::k_grappler
#include "docs/aten_xpu/plugin_entry.h"

import std;
import aten_xpu;

extern "C"
{
    ATEN_XPU_PLUGIN_ENTRY("aten_xpu.grappler")

    ATEN_XPU_EXPORT_OPS(
        grappler_device_graph,
        TFGrapplerDeviceGraphOps,
        registry.getOps().getDeviceGraphOps()
    )
    ATEN_XPU_EXPORT_OPS(grappler, TF_GrapplerOps, &registry.getGrapplerOps())
    ATEN_XPU_EXPORT_OPS(grappler_optimizer, TFGrapplerOptimizerOps, &registry.getOptimizerOps())
    ATEN_XPU_EXPORT_OPS(grappler_configs, TFGrapplerConfigsOps, &registry.getConfigsOps())
}
