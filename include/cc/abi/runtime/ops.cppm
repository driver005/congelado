module;

#include "c/extern/ops.h"

export module cc_abi_runtime:ops;

export namespace ice {

// OpsRuntime — host-side placeholder for the ops ABI. `ops.h` is registration-only: there is
// no getter to retrieve a registered TF_ShapeInferenceFn back from the host's op registry
// (TF_RegisterOpDefinition consumes the builder with no return value), and no host-side
// consumer exists in this repo yet. Kept minimal until a real invocation surface exists.
class OpsRuntime {
 public:
  OpsRuntime() = default;
};

}  // namespace ice
