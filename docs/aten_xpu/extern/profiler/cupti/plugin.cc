#include "docs/aten_xpu/extern/profiler/common/vendor_entry.h"
#include "include/c/extern/profiler/profiler.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"

import std;
import aten_xpu_profiler_common;
import aten_xpu_profiler_cupti;

ATEN_XPU_EXPORT_VENDOR_PROFILER(aten_xpu::CuptiBackend, "GPU")
