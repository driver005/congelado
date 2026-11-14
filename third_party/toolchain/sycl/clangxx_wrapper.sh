#!/usr/bin/env bash
# The DPC++ clang++. A compile that carries -DCONGELADO_SERIAL_COMPILE (congelado_module_library(serial = True))
# takes a machine-wide lock first, so such targets compile one file at a time while all others stay parallel.
set -euo pipefail

compiler="${SYCL_CLANGXX:-external/+third_party_deps+sycl/bin/clang++}"

for argument in "$@"; do
    if [[ "$argument" == "-DCONGELADO_SERIAL_COMPILE" ]]; then
        exec flock "${TMPDIR:-/tmp}/congelado-serial-compile.lock" "$compiler" "$@"
    fi
done

exec "$compiler" "$@"
