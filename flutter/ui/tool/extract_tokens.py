#!/usr/bin/env python3
"""Extract HeroUI v3 theme tokens from the pinned @heroui/styles CSS source.

Stage 1 of the token pipeline (extract -> normalize -> generate -> verify).

Input : tool/sources/heroui-variables.css  (pinned @heroui/styles@3.2.4
         themes/default/variables.css, see SHA256SUMS)
Output: tool/build/raw-tokens.json          (gitignored scratch)

This script resolves, per theme (light/dark):
  * `--name: value;` custom properties from the `:root/.light` and `.dark`
    blocks of the pinned variables.css;
  * `var(--x)` / `var(--x, fallback)` references (recursively);
  * `oklch(...)` colors -> sRGB 0xAARRGGBB;
  * `rgba(...)` (comma and CSS Color 4 slash syntax) -> 0xAARRGGBB;
  * `color-mix(in oklab, A p%, B q%)` -> premultiplied OKLab interpolation,
    converted back to straight-alpha sRGB;
  * `calc(var(--radius) * 1.5)` -> double;
  * `0.25rem` -> 16px-based logical px;
  * shadow lists -> [{color, offsetX, offsetY, blur, spread}].

Everything is deterministic: no randomness, no clock; float formatting is
rounded to 4 decimals. Colors are emitted as 8-digit 0xAARRGGBB strings.

Only the *variables* file is consumed here (tier-1 automated extraction).
Component-level measurements (heights, paddings, per-component radii) are a
tier-2/3 transcription of the component CSS, authored with citations in
tool/authored/hero-component-tokens.json and merged in the normalize stage.
"""
from __future__ import annotations

import json
import math
import re
import sys
from pathlib import Path

TOOL = Path(__file__).resolve().parent
SOURCES = TOOL / "sources"
VARIABLES_CSS = SOURCES / "heroui-variables.css"
OUT = TOOL / "build" / "raw-tokens.json"

# ---------------------------------------------------------------------------
# Color math: OKLCH <-> OKLab <-> sRGB (D65), per CSS Color 4 / Color 5.
# ---------------------------------------------------------------------------


def _srgb_to_linear(c: float) -> float:
    c = min(max(c, 0.0), 1.0)
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4


def _linear_to_srgb(c: float) -> float:
    c = min(max(c, 0.0), 1.0)
    return c * 12.92 if c <= 0.0031308 else 1.055 * (c ** (1 / 2.4)) - 0.055


def srgb_to_oklab(r: float, g: float, b: float) -> tuple[float, float, float]:
    r, g, b = _srgb_to_linear(r), _srgb_to_linear(g), _srgb_to_linear(b)
    l = 0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b
    m = 0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b
    s = 0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b
    l_, m_, s_ = l ** (1 / 3), m ** (1 / 3), s ** (1 / 3)
    return (
        0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_,
        1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_,
        0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_,
    )


def oklab_to_srgb(L: float, a: float, b: float) -> tuple[float, float, float]:
    l_ = L + 0.3963377774 * a + 0.2158037573 * b
    m_ = L - 0.1055613458 * a - 0.0638541728 * b
    s_ = L - 0.0894841775 * a - 1.2914855480 * b
    l, m, s = l_ ** 3, m_ ** 3, s_ ** 3
    r = 4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s
    g = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s
    b = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s
    return _linear_to_srgb(r), _linear_to_srgb(g), _linear_to_srgb(b)


def oklch_to_srgb(L: float, C: float, H: float) -> tuple[float, float, float]:
    """OKLCH (L in [0,1], C, H degrees) -> sRGB (0..1 each)."""
    a = C * math.cos(math.radians(H))
    b = C * math.sin(math.radians(H))
    return oklab_to_srgb(L, a, b)


def color_to_oklab_alpha(color: tuple[float, float, float, float]):
    """sRGB+alpha -> (L, a, b, alpha) in OKLab."""
    r, g, b, alpha = color
    L, a, b = srgb_to_oklab(r, g, b)
    return (L, a, b, alpha)


def oklab_alpha_to_color(L: float, a: float, b: float, alpha: float):
    r, g, b = oklab_to_srgb(L, a, b)
    return (r, g, b, alpha)


def mix_colors(
    c1: tuple[float, float, float, float],
    w1: float,
    c2: tuple[float, float, float, float],
    w2: float,
):
    """color-mix in oklab, premultiplied-alpha interpolation (CSS Color 5).

    Both weights are the raw percentages; they are normalized to sum 100%.
    """
    total = w1 + w2
    if total <= 0:
        raise ValueError("color-mix weights sum to zero")
    w1, w2 = w1 / total, w2 / total
    L1, a1, b1, al1 = color_to_oklab_alpha(c1)
    L2, a2, b2, al2 = color_to_oklab_alpha(c2)
    # Premultiply by alpha, interpolate, then unpremultiply.
    al = w1 * al1 + w2 * al2
    if al <= 1e-9:
        return (0.0, 0.0, 0.0, 0.0)
    L = (w1 * al1 * L1 + w2 * al2 * L2) / al
    a = (w1 * al1 * a1 + w2 * al2 * a2) / al
    b = (w1 * al1 * b1 + w2 * al2 * b2) / al
    return oklab_alpha_to_color(L, a, b, al)


TRANSPARENT = (0.0, 0.0, 0.0, 0.0)


def color_to_hex(color: tuple[float, float, float, float]) -> str:
    r, g, b, a = color
    r8 = round(min(max(r, 0.0), 1.0) * 255)
    g8 = round(min(max(g, 0.0), 1.0) * 255)
    b8 = round(min(max(b, 0.0), 1.0) * 255)
    a8 = round(min(max(a, 0.0), 1.0) * 255)
    return f"0x{a8:02X}{r8:02X}{g8:02X}{b8:02X}"


# ---------------------------------------------------------------------------
# Value parsing helpers
# ---------------------------------------------------------------------------

_NUM = r"[-+]?(?:\d+\.?\d*|\.\d+)"
_PCT = rf"(?:{_NUM}%?)"
_RGBA_COMMA = re.compile(
    rf"rgba?\(\s*({_NUM})\s*[,]\s*({_NUM})\s*[,]\s*({_NUM})\s*(?:[,/]\s*({_PCT})\s*)?\)"
)
_RGBA_SPACE = re.compile(
    rf"rgba?\(\s*({_NUM})\s+({_NUM})\s+({_NUM})\s*(?:/\s*({_PCT})\s*)?\)"
)
_OKLCH = re.compile(
    rf"oklch\(\s*({_PCT})\s+({_PCT})\s+({_PCT})\s*(?:/\s*({_PCT})\s*)?\)"
)
_COLOR_MIX = re.compile(
    rf"color-mix\(\s*in\s+(?:oklab|oklch)\s*,\s*(.+?)\s*,\s*(.+?)\s*\)"
)
_VAR_START = re.compile(r"var\(")
_CALC = re.compile(r"calc\(([^)]*)\)")


def parse_alpha(value: str) -> float:
    value = value.strip()
    if value.endswith("%"):
        return float(value[:-1]) / 100.0
    return float(value)


def parse_oklch_component(value: str) -> float:
    """Parse an OKLCH component; accepts fractional and percentage forms."""
    value = value.strip()
    if value.endswith("%"):
        return float(value[:-1]) / 100.0
    return float(value)


def scan_var(text: str, start: int = 0):
    """Find the next `var(...)` with balanced parens at/after [start].

    Returns (ref_name_without_dashes, fallback_or_None, match_start, end_after_close)
    or None. Handles nested parens inside the fallback (`var(--a, var(--b))`).
    """
    m = _VAR_START.search(text, start)
    if not m:
        return None
    i = m.end() - 1  # index of '('
    depth = 1
    k = i + 1
    comma = -1
    while k < len(text) and depth > 0:
        ch = text[k]
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        elif ch == "," and depth == 1 and comma < 0:
            comma = k
        k += 1
    close = k - 1
    if comma < 0:
        name = text[i + 1:close].strip()
        fallback = None
    else:
        name = text[i + 1:comma].strip()
        fallback = text[comma + 1:close].strip()
    if name.startswith("--"):
        name = name[2:]
    return name, fallback, m.start(), close + 1


def hex_to_color(value: str) -> tuple[float, float, float, float]:
    """0xAARRGGBB string -> (r, g, b, a) each 0..1."""
    if not re.fullmatch(r"0x[0-9A-Fa-f]{8}", value):
        raise ValueError(f"invalid hex color: {value!r}")
    v = int(value, 16)
    a = ((v >> 24) & 0xFF) / 255.0
    r = ((v >> 16) & 0xFF) / 255.0
    g = ((v >> 8) & 0xFF) / 255.0
    b = (v & 0xFF) / 255.0
    return (r, g, b, a)


class CssResolver:
    """Resolves custom-property values within one theme block."""

    def __init__(self, props: dict[str, str]):
        self.props = props
        self._cache: dict[str, object] = {}
        self._resolving: set[str] = set()

    # -- raw value resolution -------------------------------------------------

    def resolve(self, name: str) -> object:
        if name in self._cache:
            return self._cache[name]
        if name in self._resolving:
            raise ValueError(f"circular var() reference: --{name}")
        self._resolving.add(name)
        try:
            value = self.props[name]
            out = self._resolve_value(value)
        except KeyError:
            raise KeyError(f"unknown custom property: --{name}") from None
        finally:
            self._resolving.discard(name)
        self._cache[name] = out
        return out

    def _resolve_value(self, value: str) -> object:
        value = value.strip()
        if not value:
            raise ValueError("empty custom property value")
        # var() references with optional (possibly nested) fallbacks — resolved
        # left to right, outermost first, then re-parsed until no var() remains.
        while True:
            found = scan_var(value)
            if found is None:
                break
            ref, fallback, start, end = found
            if ref in self.props:
                resolved = self.resolve(ref)
            elif fallback:
                resolved = self._resolve_value(fallback)
            else:
                raise KeyError(f"unknown custom property: --{ref}")
            value = value[:start] + str(resolved) + value[end:]
            value = value.strip()
        # color-mix
        m = _COLOR_MIX.match(value)
        if m:
            left, right = m.group(1), m.group(2)
            return self._resolve_color_mix(left, right)
        # calc()
        m = _CALC.match(value)
        if m:
            return self._resolve_calc(m.group(1))
        # colors
        if value.startswith("oklch"):
            m = _OKLCH.match(value)
            if not m:
                raise ValueError(f"unparseable oklch: {value!r}")
            L = parse_oklch_component(m.group(1))
            C = parse_oklch_component(m.group(2))
            H = parse_oklch_component(m.group(3))
            alpha = parse_alpha(m.group(4)) if m.group(4) else 1.0
            r, g, b = oklch_to_srgb(L, C, H)
            return color_to_hex((r, g, b, alpha))
        if value.startswith("rgba") or value.startswith("rgb"):
            m = _RGBA_COMMA.match(value) or _RGBA_SPACE.match(value)
            if not m:
                raise ValueError(f"unparseable rgb: {value!r}")
            r, g, b = (float(m.group(i)) / 255.0 for i in (1, 2, 3))
            alpha = parse_alpha(m.group(4)) if m.group(4) else 1.0
            return color_to_hex((r, g, b, alpha))
        if value.startswith("0x"):
            return value
        # lengths / unitless numbers
        if value.endswith("rem"):
            return round(float(value[:-3]) * 16.0, 4)
        if value.endswith("px"):
            return round(float(value[:-2]), 4)
        if value.endswith("ms"):
            return int(round(float(value[:-2])))
        if value.endswith("s"):
            return int(round(float(value[:-1]) * 1000))
        if re.fullmatch(_NUM, value):
            return round(float(value), 4)
        # bare keywords that stay symbolic
        if value == "transparent":
            return "0x00000000"
        if re.fullmatch(r"[a-z-]+", value):
            return value
        raise ValueError(f"unparseable value: {value!r}")

    def _resolve_color_mix(self, left: str, right: str) -> str:
        def split(color_part: str) -> tuple[tuple, float | None]:
            """Return (color, weight_percent_or_None).

            A missing percentage means "fill the remainder of 100%" (CSS
            Color 5 color-mix inference), not 100%.
            """
            color_part = color_part.strip()
            m = re.search(rf"({_NUM})%\s*$", color_part)
            if m:
                weight = float(m.group(1))
                color_str = color_part[: m.start()].strip()
            else:
                weight = None
                color_str = color_part
            if color_str == "transparent":
                color = TRANSPARENT
            elif color_str.startswith("0x"):
                color = hex_to_color(color_str)
            else:
                m = _OKLCH.match(color_str)
                if not m:
                    raise ValueError(f"color-mix operand not oklch/hex/transparent: {color_str!r}")
                L = parse_oklch_component(m.group(1))
                C = parse_oklch_component(m.group(2))
                H = parse_oklch_component(m.group(3))
                alpha = parse_alpha(m.group(4)) if m.group(4) else 1.0
                r, g, b = oklch_to_srgb(L, C, H)
                color = (r, g, b, alpha)
            return color, weight

        operands = [split(left), split(right)]
        known_sum = sum(w for _, w in operands if w is not None)
        missing_count = sum(1 for _, w in operands if w is None)
        if missing_count > 0:
            share = max(100.0 - known_sum, 0.0) / missing_count
            weights = [w if w is not None else share for _, w in operands]
        else:
            weights = [w for _, w in operands]
        colors = [c for c, _ in operands]
        return color_to_hex(mix_colors(colors[0], weights[0], colors[1], weights[1]))

    def _resolve_calc(self, expr: str) -> float:
        expr = expr.replace("var(--radius)", str(self.resolve("radius")))
        expr = expr.replace("rem", "").replace("px", "").strip()
        if re.fullmatch(rf"\(?{_NUM}\)?", expr):
            return round(float(expr.strip("()")), 4)
        # Only support `X * Y` and `X / Y` shapes actually used by HeroUI.
        for op in ("*", "/"):
            if op in expr:
                a, b = expr.split(op)
                a = float(a.strip().strip("()"))
                b = float(b.strip().strip("()"))
                return round(a * b if op == "*" else a / b, 4)
        raise ValueError(f"unsupported calc(): {expr!r}")

    # -- shadow lists ---------------------------------------------------------

    def resolve_shadows(self, name: str) -> list[dict]:
        """Parse a box-shadow list value (vars already resolved)."""
        raw = self.props[name]
        try:
            resolved = self._resolve_value(raw)
        except ValueError:
            resolved = None  # shadow list, not a single color — parse below
        if isinstance(resolved, list):
            return resolved
        if isinstance(resolved, str) and resolved.startswith("0x"):
            return [{"color": resolved, "offsetX": 0.0, "offsetY": 0.0, "blur": 0.0, "spread": 0.0}]
        # Not a color: parse the shadow list from the raw text with vars resolved.
        text = raw
        while True:
            found = scan_var(text)
            if found is None:
                break
            ref, fallback, start, end = found
            rep = self.resolve(ref) if ref in self.props else (fallback.strip() if fallback else "")
            text = text[:start] + str(rep) + text[end:]
        entries: list[dict] = []
        for part in self._split_shadow_list(text):
            entries.append(self._parse_shadow(part))
        return entries

    def _split_shadow_list(self, text: str) -> list[str]:
        parts, depth, cur = [], 0, ""
        for ch in text:
            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
            if ch == "," and depth == 0:
                parts.append(cur)
                cur = ""
            else:
                cur += ch
        if cur.strip():
            parts.append(cur)
        return [p.strip() for p in parts if p.strip()]

    def _parse_shadow(self, part: str) -> dict:
        # color may lead or trail; numbers are offset-x offset-y blur spread.
        # Dark HeroUI shadows use `inset` — Flutter has no inset shadows, so the
        # keyword is dropped (documented approximation in the worksheets).
        part = part.replace("inset", " ").strip()
        color = None
        m = re.search(r"(0x[0-9A-F]{8}|rgba?\([^)]*\))", part)
        if m:
            color = self._resolve_value(m.group(1))
            part = part[: m.start()] + part[m.end():]
        nums = [float(x) for x in re.findall(rf"{_NUM}", part)]
        if len(nums) < 2:
            raise ValueError(f"unparseable shadow: {part!r}")
        nums = nums[:4]
        while len(nums) < 4:
            nums.append(0.0)
        if color is None:
            # `transparent` keyword (dark theme "no shadow" value) -> alpha 0.
            color = "0x00000000" if "transparent" in part else "0xFF000000"
        return {
            "color": color,
            "offsetX": nums[0],
            "offsetY": nums[1],
            "blur": nums[2],
            "spread": nums[3],
        }


def extract_blocks(css: str):
    """Return {selector: {prop: value}} for the theme blocks.

    The pinned variables.css wraps its blocks in `@layer base { ... }`, so the
    theme blocks (`:root, .light, ...` and `.dark, ...`) sit at depth 1. This
    parser records every block at depth 1, capturing its selector and raw text
    (nested braces are balanced; color-mix() contains parens, not braces, so a
    brace counter suffices).
    """
    blocks: dict[str, dict[str, str]] = {}
    depth = 0
    selector = ""
    current: dict[str, str] | None = None
    buf = ""
    for ch in css:
        if ch == "{":
            if depth == 1 and current is None and selector:
                current = {}
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 1 and current is not None:
                blocks[selector] = current
                current = None
                selector = ""
            buf = ""
        else:
            if depth == 1 and current is None:
                selector += ch
            elif depth >= 2 and current is not None:
                current.setdefault("_raw", "")
                current["_raw"] += ch
            else:
                buf += ch
    return blocks


def parse_props(raw: str) -> dict[str, str]:
    props: dict[str, str] = {}
    # strip comments
    raw = re.sub(r"/\*.*?\*/", "", raw, flags=re.S)
    # split on top-level ';'
    depth = 0
    cur = ""
    for ch in raw:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == ";" and depth == 0:
            cur = cur.strip()
            if cur.startswith("--"):
                name, _, value = cur.partition(":")
                props[name.strip()[2:]] = value.strip()
            cur = ""
        else:
            cur += ch
    return props


def main() -> int:
    css = VARIABLES_CSS.read_text()
    blocks = extract_blocks(css)
    light_props = dark_props = None
    for selector, raw in blocks.items():
        if ":root" in selector or "data-theme=\"light\"" in selector:
            light_props = parse_props(raw["_raw"])
        elif "data-theme=\"dark\"" in selector and "vibrant" not in selector:
            dark_props = parse_props(raw["_raw"])

    if light_props is None or dark_props is None:
        print("ERROR: could not locate light and dark theme blocks", file=sys.stderr)
        return 1

    light = CssResolver(light_props)
    # The .dark block sits inside the same cascade as :root, so custom
    # properties it does not re-declare (primitives like --snow/--eclipse,
    # plus --accent/--success) inherit the light block's values.
    dark = CssResolver({**light_props, **dark_props})

    # All custom property names from the light block (dark inherits missing).
    names = sorted(light_props.keys())
    shadow_names = {"surface-shadow", "overlay-shadow", "field-shadow"}
    raw: dict[str, dict] = {"themes": ["light", "dark"], "variables": {}}
    for name in names:
        if name in shadow_names:
            continue
        entry: dict = {"light": None, "dark": None}
        for theme, resolver in (("light", light), ("dark", dark)):
            try:
                value = resolver.resolve(name)
            except Exception as e:  # noqa: BLE001 - record for inventory
                if theme == "light":
                    print(f"  !! --{name} [{theme}]: {e}", file=sys.stderr)
                continue
            entry[theme] = value
        raw["variables"][name] = entry

    # Shadows are lists, resolved separately.
    for name in ("surface-shadow", "overlay-shadow", "field-shadow"):
        raw["variables"][name] = {
            "light": light.resolve_shadows(name),
            "dark": dark.resolve_shadows(name),
        }

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(raw, indent=2, sort_keys=True) + "\n")
    count = len(raw["variables"])
    print(f"extract: {count} custom properties -> {OUT.relative_to(TOOL.parent.parent)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
