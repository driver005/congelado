#!/usr/bin/env bash
# GC tripwire: fail build if GC allocation symbols leak in.
# Usage: ./scripts/gc_tripwire.sh <binary>
set -euo pipefail
BINARY="${1:?Usage: gc_tripwire.sh <binary>}"
if nm -u "$BINARY" | grep -qE '_d_(newclass|newitemT|arrayliteralTX|arrayappend|allocmemory|assocarray)'; then
    echo "GC TRIPWIRE FAILED: GC symbols found in $BINARY" >&2
    nm -u "$BINARY" | grep -E '_d_(newclass|newitemT|arrayliteralTX|arrayappend|allocmemory|assocarray)' >&2
    exit 1
fi
echo "GC tripwire: PASS (no GC symbols in $BINARY)"
