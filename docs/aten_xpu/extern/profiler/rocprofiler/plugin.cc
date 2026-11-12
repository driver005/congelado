#include "docs/aten_xpu/extern/profiler/common/vendor_entry.h"
#include "include/c/extern/profiler/profiler.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"

#if __has_include(<rocprofiler-sdk/registration.h>)
    #include <rocprofiler-sdk/registration.h>
#endif

import std;
import aten_xpu_profiler_common;
import aten_xpu_profiler_rocprofiler;

ATEN_XPU_EXPORT_VENDOR_PROFILER(aten_xpu::RocprofilerBackend, "GPU")

#if __has_include(<rocprofiler-sdk/registration.h>)
// rocprofiler-sdk finds this symbol in loaded libraries when the HSA runtime starts.
extern "C" TF_CAPI_EXPORT rocprofiler_tool_configure_result_t* rocprofiler_configure(
    uint32_t version,
    const char* runtime_version,
    uint32_t priority,
    rocprofiler_client_id_t* client_id
)
{
    return aten_xpu::RocprofilerBackend::configure(version, runtime_version, priority, client_id);
}
#endif
