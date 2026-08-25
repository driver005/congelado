#!/usr/bin/env python3
"""Static audit for the kowalski component layer.

Runs the checks that guard the HeroUI-mirror rules without needing the
Flutter SDK:

  1. No forbidden design colors (colorWhite/colorBlack/colorSnow,
     Colors.*, Color(0x...), indigo) in lib/.
  2. Every HeroTokens.<name> / heroEase* reference resolves to a real
     handle/curve in the generated token file.
  3. Paren/brace/bracket balance per file.
  4. Parity (delegates to tool/check_parity.py).

Exit 0 == clean.
"""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent  # flutter/ui

FORBIDDEN = re.compile(r"colorWhite|colorBlack|colorSnow|\bColors\.|Color\(0x|indigo")

# Generated token files legitimately contain the primitive handles
# (colorWhite/colorBlack/colorSnow — present in the registry but FORBIDDEN in
# component code) and raw 0xAARRGGBB values. Only authored code is audited.
EXCLUDE = {"hero_tokens.g.dart"}

SCAN_DIRS = [ROOT / "lib"]


def token_members() -> tuple[set[str], set[str]]:
    tok = (ROOT / "lib" / "src" / "tokens" / "generated" / "hero_tokens.g.dart").read_text()
    members = set(re.findall(r"static const \w+ (\w+) =", tok))
    eases = set(re.findall(r"const Cubic (\w+) =", tok)) | {"heroEaseOutQuart"}
    return members, eases


def main() -> int:
    members, eases = token_members()
    problems: list[str] = []

    files = sorted(
        p for d in SCAN_DIRS for p in d.rglob("*.dart")
        if p.name not in EXCLUDE
    )

    for f in files:
        s = f.read_text()

        for m in FORBIDDEN.finditer(s):
            problems.append(f"{f.relative_to(ROOT)}: forbidden {m.group(0)!r}")

        for m in re.findall(r"HeroTokens\.(\w+)", s):
            if m not in members:
                problems.append(f"{f.relative_to(ROOT)}: unknown token HeroTokens.{m}")

        for m in re.findall(r"heroEase\w+", s):
            if m not in eases:
                problems.append(f"{f.relative_to(ROOT)}: unknown curve {m}")

        for a, b in [("(", ")"), ("[", "]"), ("{", "}")]:
            if s.count(a) != s.count(b):
                problems.append(
                    f"{f.relative_to(ROOT)}: unbalanced {a}{b} "
                    f"({s.count(a)} vs {s.count(b)})"
                )

    for p in problems:
        print(f"[static] {p}")

    # Parity check.
    parity = subprocess.run(
        [sys.executable, str(ROOT / "tool" / "check_parity.py")],
        capture_output=True, text=True,
    )
    print(parity.stdout, end="")
    if parity.returncode != 0:
        problems.append("parity gaps (see above)")

    print(f"\n[static] {len(files)} files, {len(problems)} problems")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
