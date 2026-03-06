#!/usr/bin/env bash
set -e

BINARY=./bazel-bin/freelist_test

echo "=== Helgrind ==="
# valgrind \
#   --tool=helgrind \
#   --history-level=approx \
#   --free-is-write=yes \
#   "$BINARY" 2>&1 | tee helgrind.out

echo "=== Massif ==="
valgrind \
  --tool=massif \
  --pages-as-heap=no \
  "$BINARY"
ms_print massif.out."$(find -t massif.out.* | head -1 | grep -o '[0-9]*$')" | head -60
