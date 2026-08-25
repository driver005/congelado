module;

#include "c/extern/env/time.h"

export module cc_abi_builder_env:time;

import cc_abi_builder_intern;

export namespace ice {

class Time {
 public:
  static uint64_t now_nanos() { return TF_NowNanos(); }
  static uint64_t now_micros() { return TF_NowMicros(); }
  static uint64_t now_seconds() { return TF_NowSeconds(); }
};

}  // namespace ice
