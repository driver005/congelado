// Isolated probe: does Bazel's module scanner handle #include inside a global module fragment (matches include/core/manager/ffi.cppm's pattern)?
module;

#include <congelado/abi.h>

export module gmf_probe;

import std;

export namespace gmf_probe {
// touches a real abi.h symbol so the #include can't be dead-stripped away
constexpr auto kProbeRunType = CONGELADO_RUN_LOGGER;
}
