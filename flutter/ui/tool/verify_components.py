#!/usr/bin/env python3
"""Read-only verification of the extracted component specs.

Regenerates tool/generated/component-specs.json into a temp file via --out and
diffs byte-for-byte against the committed one. Pure stdlib, no network.
"""
from __future__ import annotations

import subprocess
import sys
import tempfile
from pathlib import Path

TOOL = Path(__file__).resolve().parent
COMMITTED = TOOL / "generated" / "component-specs.json"


def main() -> int:
    with tempfile.TemporaryDirectory() as tmp:
        out = Path(tmp) / "component-specs.json"
        proc = subprocess.run(
            [sys.executable, "tool/extract_components.py", f"--out={out}"],
            cwd=TOOL.parent,
            capture_output=True,
            text=True,
        )
        if proc.returncode != 0:
            print("extract-components FAILED")
            print(proc.stderr[-2000:])
            return 1
        if out.read_bytes() != COMMITTED.read_bytes():
            print("FAIL: component-specs.json drifted from pinned component CSS")
            return 1
        print("verify-components: byte-identical, ok")
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
