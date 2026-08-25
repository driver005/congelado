// Compile-time guard for the OpenXLA C ABI umbrella (see BUILD.bazel).
// Touches one symbol per surface area; never executed, only compiled.

#include <stddef.h>

#include "xla/xla.h"

int main(void) {
  const PJRT_Api* api = NULL;
  const XLA_FFI_CallFrame* frame = NULL;
  TSL_Status* status = NULL;
  (void)api;
  (void)frame;
  (void)status;
  return PJRT_Error_Code_OK == 0 && TSL_OK == 0 ? 0 : 1;
}
