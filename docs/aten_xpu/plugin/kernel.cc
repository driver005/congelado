#include "include/c/extern/plugin/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#define ATEN_XPU_DOMAIN aten_xpu::SyclPluginDomain::k_kernel
#include "docs/aten_xpu/plugin_entry.h"

import std;
import aten_xpu;

extern "C"
{
    ATEN_XPU_PLUGIN_ENTRY("aten_xpu.kernel")
}
