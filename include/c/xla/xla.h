/* Complete OpenXLA C ABI layer for congelado.
 *
 * Umbrella header — pulls in the full pure-C surface of OpenXLA (PJRT C API,
 * all extensions, XLA FFI handler API, tsl status, shared enums). The headers
 * themselves are NOT copied here; they resolve from the http_archive-fetched
 * XLA source (@xla//). The include path is set by the XLA module's BUILD files.
 *
 * Provenance: third_party/xla/repo.bzl (pin + fetch policy).
 */

#ifndef CONGELADO_C_XLA_XLA_H_
#define CONGELADO_C_XLA_XLA_H_

/* ------------------------------- PJRT core -------------------------------- */
#include "xla/pjrt/c/pjrt_c_api.h"
#include "xla/pjrt/c/pjrt_c_api_macros.h"

/* ------------------- PJRT plugin entries & shared types ------------------- */
#include "xla/pjrt/c/pjrt_c_api_cpu.h"
#include "xla/pjrt/c/pjrt_c_api_gpu.h"
#include "xla/pjrt/c/pjrt_c_api_tpu.h"
#include "xla/pjrt/c/pjrt_c_api_tpu_constants.h"
#include "xla/pjrt/c/pjrt_c_api_device_event.h"

/* ---------------------------- PJRT extensions ----------------------------- */
#include "xla/pjrt/c/pjrt_c_api_abi_version_extension.h"
#include "xla/pjrt/c/pjrt_c_api_callback_extension.h"
#include "xla/pjrt/c/pjrt_c_api_collectives_extension.h"
#include "xla/pjrt/c/pjrt_c_api_custom_partitioner_extension.h"
#include "xla/pjrt/c/pjrt_c_api_ffi_extension.h"
#include "xla/pjrt/c/pjrt_c_api_gpu_extension.h"
#include "xla/pjrt/c/pjrt_c_api_layouts_extension.h"
#include "xla/pjrt/c/pjrt_c_api_megascale_extension.h"
#include "xla/pjrt/c/pjrt_c_api_memory_descriptions_extension.h"
#include "xla/pjrt/c/pjrt_c_api_multi_slice_extension.h"
#include "xla/pjrt/c/pjrt_c_api_phase_compile_extension.h"
#include "xla/backends/profiler/plugin/profiler_c_api.h"
#include "xla/pjrt/c/pjrt_c_api_profiler_extension.h"
#include "xla/pjrt/c/pjrt_c_api_raw_buffer_extension.h"
#include "xla/pjrt/c/pjrt_c_api_shardings_extension.h"
#include "xla/pjrt/c/pjrt_c_api_stream_extension.h"
#include "xla/pjrt/c/pjrt_c_api_tpu_executable_extension.h"
#include "xla/pjrt/c/pjrt_c_api_tpu_topology_extension.h"
#include "xla/pjrt/c/pjrt_c_api_triton_extension.h"
#include "xla/pjrt/c/pjrt_c_api_xla_transform_extension.h"

/* ----------------------------- FFI handler side --------------------------- */
#include "xla/ffi/api/c_api.h"
#include "xla/ffi/api/collectives_c_api.h"
#include "xla/ffi/api/record_c_api.h"

/* --------------------------- Status + shared enums ------------------------ */
#include "xla/tsl/c/tsl_status.h"
#include "xla/c/c_api_decl.h"

#endif /* CONGELADO_C_XLA_XLA_H_ */
