#!/usr/bin/env python3
"""Extract HeroUI v3 component specs mechanically from the pinned component CSS.

Stage 1.5 of the pipeline: turns `tool/sources/heroui-*.css` (the @apply
class lists + raw declarations from @heroui/styles@3.2.4) into a structured,
cited component spec — replacing hand-transcription as the source of truth
for component geometry/colors/states.

Output: tool/generated/component-specs.json  (COMMITTED)
Verify : tool/verify_components.py            (read-only byte-diff)

Tailwind v4 class resolution rules implemented (only the class set actually
used by the pinned files):
  * spacing utilities (h-* w-* px-* py-* p-* gap-* size-* min-h-* min-w-*
    max-w-* m-* ms-* mt-* mb-* top-* start-* end-* left-* w-px h-px) =
    value * 0.25rem = 4px scale (fractions and negatives handled);
  * text-* font sizes + leading-* line heights (Tailwind v4 fixed table);
  * rounded-* -> HeroUI --radius-* scale (xs2 sm4 md6 lg8 xl12 2xl16 3xl24
    4xl32 full9999);
  * font-medium/semibold/bold -> 500/600/700;
  * border-* widths -> 1px (border-2 -> 2px, border-0 -> 0);
  * bg-*/text-*/border-* color classes -> CSS custom property references
    (resolved against the theme variables where resolvable);
  * size-* -> both width+height;
  * `md:`/`sm:` responsive prefixes -> recorded under `md:`/`sm` keys,
    with the *desktop* (`md:`/base) value marked as the render target.
"""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

TOOL = Path(__file__).resolve().parent
SOURCES = TOOL / "sources"
OUT = TOOL / "generated" / "component-specs.json"

# Tailwind v4 fixed tables ---------------------------------------------------
TEXT_SIZES = {
    "xs": (12.0, 16.0), "sm": (14.0, 20.0), "base": (16.0, 24.0),
    "lg": (18.0, 28.0), "xl": (20.0, 28.0), "2xl": (24.0, 32.0),
    "3xl": (30.0, 36.0), "4xl": (36.0, 40.0), "[10px]": (10.0, None),
}
LEADING = {"4": 16.0, "5": 20.0, "6": 24.0, "7": 28.0, "8": 32.0, "9": 36.0, "10": 40.0}
RADIUS = {"xs": 2.0, "sm": 4.0, "md": 6.0, "lg": 8.0, "xl": 12.0, "2xl": 16.0,
          "3xl": 24.0, "4xl": 32.0, "full": 9999.0}
WEIGHTS = {"font-thin": 100, "font-light": 300, "font-normal": 400,
           "font-medium": 500, "font-semibold": 600, "font-bold": 700,
           "font-extrabold": 800, "font-black": 900}
SPACING_UTILS = ("h", "w", "px", "py", "p", "gap", "gap-x", "gap-y", "size",
                 "min-h", "min-w", "max-w", "max-h", "m", "mx", "my", "ms",
                 "me", "mt", "mb", "ml", "mr", "top", "start", "end", "left",
                 "bottom", "right", "inset", "inset-x", "inset-y")

_NUM = r"(?:[-+]?(?:\d+\.?\d*|\.\d+))"


def _spacing_value(token: str):
    """h-9 -> 36.0; px-0.5 -> 2.0; w-px -> 1.0; h-[3px] -> 3.0."""
    for util in SPACING_UTILS:
        prefix = f"{util}-"
        if token.startswith(prefix):
            v = token[len(prefix):]
            if v == "px":
                return util, 1.0
            if v == "full":
                return util, "full"
            if v == "fit":
                return util, "fit"
            if v == "min":
                return util, "min-content"
            m = re.fullmatch(_NUM, v)
            if m:
                return util, round(float(v) * 4.0, 4)
            m = re.fullmatch(rf"\[({_NUM})(px|rem)\]", v)
            if m:
                return util, round(float(m.group(1)) * (16.0 if m.group(2) == "rem" else 1.0), 4)
            return util, v  # unknown symbolic (e.g. w-2/5)
    return None, None


def resolve_class(cls: str):
    """Resolve one Tailwind class to (key, value, source)."""
    if cls in WEIGHTS:
        return ("fontWeight", WEIGHTS[cls], f"class:{cls}")
    m = re.fullmatch(r"text-(\[[^\]]+\]|\S+)", cls)
    if m:
        size = m.group(1)
        if size in TEXT_SIZES:
            fs, lh = TEXT_SIZES[size]
            return ("fontSize", fs, f"class:{cls}")
    m = re.fullmatch(r"leading-(\d+)", cls)
    if m:
        return ("lineHeight", LEADING[m.group(1)], f"class:{cls}")
    m = re.fullmatch(r"rounded-(\S+)", cls)
    if m:
        key = m.group(1)
        if key in RADIUS:
            return ("radius", RADIUS[key], f"class:{cls}")
    m = re.fullmatch(r"border-(\d)", cls)
    if m:
        return ("borderWidth", float(m.group(1)), f"class:{cls}")
    # size-5 -> width+height
    util, val = _spacing_value(cls)
    if util == "size" and isinstance(val, float):
        return ("size", (val, val), f"class:{cls}")
    if util and isinstance(val, float):
        return (util, val, f"class:{cls}")
    if util:
        return (util, val, f"class:{cls}")  # symbolic full/fit/etc.
    # color classes: bg-*, text-*, border-* -> css var refs kept symbolic
    m = re.match(r"(bg|text|border|ring|outline)-(color-)?(.+)$", cls)
    if m:
        return (f"color:{m.group(1)}", m.group(3), f"class:{cls}")
    return (cls, None, f"class:{cls}")  # unknown/structural class


def parse_apply(body: str, prefixed: dict):
    """Split an @apply line into classes; record resolved values (with
    responsive prefixes like md: -> the `md:` key)."""
    for cls in body.split():
        cls = cls.strip()
        if not cls or cls in (";", "transition-none", "motion-reduce:transition-none"):
            continue
        prefix = ""
        if ":" in cls:
            p, cls = cls.split(":", 1)
            prefix = p  # md / sm / motion-reduce / rtl / focus variants etc.
        key, val, src = resolve_class(cls)
        if key is None:
            continue
        target = prefixed.setdefault(prefix, {})
        if key == "size":
            target["width"], target["height"] = val, val
        else:
            target[key] = val


def extract_file(path: Path):
    css = path.read_text()
    # strip comments
    css = re.sub(r"/\*.*?\*/", "", css, flags=re.S)
    rules: dict[str, dict] = {}
    i = 0
    while True:
        j = css.find("{", i)
        if j < 0:
            break
        depth = 1
        k = j + 1
        while k < len(css) and depth > 0:
            if css[k] == "{":
                depth += 1
            elif css[k] == "}":
                depth -= 1
            k += 1
        selector = " ".join(css[i:j].split())
        body = css[j + 1:k - 1]
        i = k
        if not selector or selector.startswith(("@media", "@supports", "@keyframes", "@layer")):
            continue
        entry = rules.setdefault(selector, {"apply": {}, "props": {}, "nested": {}})
        # Top-level declarations only: text before the first nested block.
        top = body.split("{", 1)[0]
        for am in re.finditer(r"@apply\s+([^;]+);", top):
            parse_apply(am.group(1), entry["apply"])
        for dm in re.finditer(r"([a-z-]+)\s*:\s*([^;]+);", top):
            prop, value = dm.group(1).strip(), dm.group(2).strip()
            if prop.startswith("@"):
                continue
            entry["props"][prop] = value
        # Nested `& ... {}` blocks -> sub-rule keyed by the nested selector.
        for nm in re.finditer(r"([^{}]+)\{([^{}]*)\}", body):
            nsel = " ".join(nm.group(1).split())
            nbody = nm.group(2).strip()
            sub = entry["nested"].setdefault(nsel, {"apply": {}, "props": {}})
            for am in re.finditer(r"@apply\s+([^;]+);", nbody):
                parse_apply(am.group(1), sub["apply"])
            for dm in re.finditer(r"([a-z-]+)\s*:\s*([^;]+);", nbody):
                prop, value = dm.group(1).strip(), dm.group(2).strip()
                if not prop.startswith("@"):
                    sub["props"][prop] = value
    return rules


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, default=OUT)
    args = parser.parse_args()
    components = {}
    for f in sorted(SOURCES.glob("heroui-*.css")):
        if f.name == "heroui-variables.css" or f.name == "heroui-shared-theme.css":
            continue
        name = f.name.replace("heroui-", "").replace(".css", "")
        components[name] = extract_file(f)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(components, indent=1, sort_keys=True) + "\n")
    total = sum(len(v) for v in components.values())
    print(f"extract-components: {len(components)} components, {total} rules -> {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
