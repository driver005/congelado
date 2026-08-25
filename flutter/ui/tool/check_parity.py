#!/usr/bin/env python3
"""Parity check: every component in the pinned @heroui/react@3.2.4 index must
have a Hero* implementation, a worksheet and a Widgetbook use case.

The source of truth is the pinned npm tarball (extracted in
.reference/heroui-react-3.2.4). Components with multiple parts (list-box-item,
menu-section, ...) map to a single Hero* facade via ALIAS.

Exit code 1 lists every gap; exit 0 means full parity.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent  # flutter/ui
REACT = ROOT.parent.parent / ".reference" / "heroui-react-3.2.4" / "dist" / "components"

SKIP = {"index.d.ts", "index.js", "icons.d.ts", "icons.js", "rac"}

# Sub-parts that live inside a parent Hero* widget (per plan).
ALIAS = {
    "list-box-item": "HeroListBoxItem",
    "list-box-section": "HeroListBoxSection",
    "menu-item": "HeroMenuItem",
    "menu-section": "HeroMenuSection",
    "progress-bar": "HeroProgress",  # implemented as HeroProgress
    "radio-group": "HeroRadioGroup",  # grouped inside HeroRadioGroup
    "separator": "HeroDivider",  # implemented as HeroDivider (separator.css)
    "tag": "HeroTag",
    "textfield": "HeroInput",  # input.css covers both `input` and `textfield`
    "typography": "HeroTypography",
}

# Worksheet files that carry a different kebab name than the React component.
WORKSHEET_ALIAS = {"separator": "divider"}

# Components whose CSS spec is absent from @heroui/styles (container-only).
NO_SPEC = {"form"}


def hero_name(component: str) -> str:
    """hero-ui kebab-case -> Hero* CamelCase (with number/edge handling)."""
    if component in ALIAS:
        return ALIAS[component]
    parts = component.split("-")
    return "Hero" + "".join(p.capitalize() for p in parts)


def main() -> int:
    if not REACT.is_dir():
        print(f"[parity] missing source: {REACT} (run: cd .reference && "
              f"npm pack @heroui/react@3.2.4 && tar xzf ...tgz)")
        return 2

    components = sorted(
        p.name for p in REACT.iterdir()
        if p.is_dir() and p.name not in SKIP
    )

    barrel = (ROOT / "lib" / "congelado_hero_ui.dart").read_text()
    kowalski = ROOT / "lib" / "src" / "kowalski"
    use_cases_dir = ROOT / "lib" / "src" / "widgetbook" / "use_cases"
    use_cases = "\n".join(
        p.read_text() for p in use_cases_dir.glob("*.dart")
    )

    gaps: list[str] = []
    missing: list[str] = []

    for c in components:
        name = hero_name(c)
        # Implementation file (either a dedicated hero_<kebab>.dart or an
        # export line in the barrel for aliased facades). Files use
        # snake_case (`hero_close_button.dart` for `close-button`).
        file = kowalski / f"hero_{c.replace('-', '_')}.dart"
        has_impl = file.exists() or \
            f"export 'src/kowalski/hero_{c.replace('-', '_')}.dart';" in barrel
        if not has_impl:
            # aliased parts are implemented inside the parent file
            parent = ALIAS.get(c)
            if parent is not None:
                has_impl = name in use_cases or name in barrel
        if not has_impl:
            missing.append(f"{c} (-> {name})")

        # Worksheet (spec components only; form is container-only).
        worksheet_name = WORKSHEET_ALIAS.get(c, c)
        worksheet = ROOT / "specs" / "components" / f"{worksheet_name}.yaml"
        if c not in NO_SPEC and not worksheet.exists():
            gaps.append(f"[worksheet] {worksheet_name}.yaml missing")

        # Widgetbook use case mirrors the HeroUI storybook story. The
        # catalogue names components without the `Hero` prefix (e.g. the
        # `Label` component shows `HeroLabel`).
        display = name[4:] if name.startswith("Hero") else name
        if not any(
            f"'{n}'" in use_cases or f'"{n}"' in use_cases
            for n in (name, display)
        ):
            gaps.append(f"[usecase] {name} has no Widgetbook use case")

    if missing:
        print("[parity] MISSING implementations:")
        for m in missing:
            print(f"  - {m}")

    if gaps:
        print("[parity] GAPS:")
        for g in gaps:
            print(f"  - {g}")

    total = len(components)
    done = total - len(missing)
    print(f"\n[parity] {done}/{total} components implemented "
          f"({len(gaps)} worksheet/usecase gaps)")
    return 1 if (missing or gaps) else 0


if __name__ == "__main__":
    sys.exit(main())
