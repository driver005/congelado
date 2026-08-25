#!/usr/bin/env python3
"""Read-only verification of the generated token artifacts.

Stage 4 of the token pipeline (extract -> normalize -> generate -> verify).

Read-only: every writer stage is invoked with `--out` into a temp dir, and
the committed files are only ever *compared*, never overwritten. Suitable for
CI without any dependency install (pure Python + stdlib).

Checks:
  1. normalize_tokens regenerates byte-identically from the pinned sources;
  2. generate_tokens regenerates byte-identically from the committed snapshot;
  3. every generated file carries the provenance header;
  4. no unparsed CSS units (rem/vw/em, cubic-bezier, rgb()/rgba()/oklch())
     survive on code lines;
  5. every emitted color matches ^0x[0-9A-F]{8}$.

Exit code 0 = all stages pass; 1 = any failure.
"""
from __future__ import annotations

import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path

TOOL = Path(__file__).resolve().parent
PKG = TOOL.parent
SNAPSHOT = TOOL / "tokens" / "hero-tokens.normalized.json"
GENERATED = PKG / "lib" / "src" / "tokens" / "generated" / "hero_tokens.g.dart"

HEADER = "GENERATED FILE — DO NOT EDIT BY HAND"
UNPARSED = re.compile(r"\b\d+(?:\.\d+)?(?:rem|vw|em)\b|cubic-bezier|oklch\(|rgba?\(")
COLOR = re.compile(r"^0x[0-9A-F]{8}$")
failures = 0


def stage(name: str) -> None:
    print(f"[verify] {name} ... ", end="", flush=True)


def ok() -> None:
    print("ok")


def fail() -> None:
    global failures
    failures += 1
    print("FAIL")


def run(args: list[str], cwd: Path) -> subprocess.CompletedProcess:
    return subprocess.run([sys.executable, *args], cwd=cwd, capture_output=True, text=True)


def main() -> int:
    global failures

    # 1. Re-run extract + normalize in a scratch copy of tool/ (sans build/)
    #    and diff the regenerated snapshot against the committed one — read-only.
    stage("extract+normalize reproducible")
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        scratch = tmp / "tool"
        scratch.mkdir(parents=True)
        for item in TOOL.iterdir():
            if item.name == "build":
                continue
            if item.is_dir():
                subprocess.run(["cp", "-R", str(item), str(scratch / item.name)], check=True)
            else:
                subprocess.run(["cp", str(item), str(scratch / item.name)], check=True)
        p1 = run(["extract_tokens.py"], cwd=scratch)
        p2 = run(["normalize_tokens.py"], cwd=scratch)
        if p1.returncode != 0 or p2.returncode != 0:
            fail()
            print(p1.stderr[-1000:], p2.stderr[-1000:])
            return 1
        regenerated = json.loads((scratch / "tokens" / "hero-tokens.normalized.json").read_text())
        committed = json.loads(SNAPSHOT.read_text())
        if regenerated != committed:
            fail()
            print("  normalized snapshot drifted from pinned sources")
            return 1
        ok()

    # 2. Generate into a temp dir and diff against the committed Dart.
    stage("generate byte-identical")
    with tempfile.TemporaryDirectory() as tmp:
        proc = run(["tool/generate_tokens.py", f"--out={tmp}"], cwd=PKG)
        if proc.returncode != 0:
            fail()
            print(proc.stderr[-2000:])
            return 1
        regenerated_dart = (Path(tmp) / "hero_tokens.g.dart").read_bytes()
        committed_dart = GENERATED.read_bytes()
        if regenerated_dart != committed_dart:
            fail()
            print("  generated Dart drifted from the committed snapshot")
            return 1
        ok()

    # 3. Provenance headers.
    stage("provenance headers")
    if HEADER in GENERATED.read_text():
        ok()
    else:
        fail()

    # 4. No unparsed CSS units / color functions on code lines.
    stage("no unparsed CSS units")
    bad = []
    for i, line in enumerate(GENERATED.read_text().splitlines(), 1):
        stripped = line.strip()
        if not stripped or stripped.startswith("//"):
            continue
        if UNPARSED.search(stripped):
            bad.append((i, stripped))
    if bad:
        fail()
        for i, line in bad[:10]:
            print(f"  line {i}: {line}")
    else:
        ok()

    # 5. Color validator.
    stage("color values valid")
    bad_colors = []
    for m in re.finditer(r"Color\((0x[0-9A-F]{8})\)", GENERATED.read_text()):
        if not COLOR.match(m.group(1)):
            bad_colors.append(m.group(1))
    if bad_colors:
        fail()
        print(f"  {bad_colors[:10]}")
    else:
        ok()

    if failures:
        print(f"\nverify: {failures} stage(s) FAILED")
        return 1
    print("\nverify: all stages passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
