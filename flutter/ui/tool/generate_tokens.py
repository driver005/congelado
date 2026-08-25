#!/usr/bin/env python3
"""Generate the Dart token file from the normalized token registry.

Stage 3 of the token pipeline (extract -> normalize -> generate -> verify).

Input : tool/tokens/hero-tokens.normalized.json  (COMMITTED)
Output: lib/src/tokens/generated/hero_tokens.g.dart  (COMMITTED, --out dir)

Emitted content:
  * provenance header (source, version, regeneration command, SPDX);
  * `HeroTokens` — static const Mix token handles (ColorToken, RadiusToken,
    SpaceToken, DoubleToken, DurationToken, FontWeightToken, TextStyleToken,
    BoxShadowToken), one per registry entry;
  * resolved value maps as TOP-LEVEL `final`s (Mix tokens override ==, so
    const maps are a compile error; top-level finals avoid per-call
    re-allocation — see the building-remix-design-system skill);
  * easing curves as const Cubic values;
  * `HeroSourceManifest` — pinned source metadata + inventory counts.

Determinism: no clock, no randomness; every emitted float prints a decimal
point; every color passes a 0xAARRGGBB validator.
"""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

TOOL = Path(__file__).resolve().parent
SNAPSHOT = TOOL / "tokens" / "hero-tokens.normalized.json"
DEFAULT_OUT = TOOL.parent / "lib" / "src" / "tokens" / "generated"

COLOR_RE = re.compile(r"^0x[0-9A-F]{8}$")
IDENT_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")

TYPE_CLASS = {
    "color": "ColorToken",
    "radius": "RadiusToken",
    "space": "SpaceToken",
    "double": "DoubleToken",
    "duration": "DurationToken",
    "fontweight": "FontWeightToken",
    "textstyle": "TextStyleToken",
    "boxshadow": "BoxShadowToken",
}

VALUE_IMPORTS = {
    "ColorToken": "Color",
    "RadiusToken": "Radius",
    "SpaceToken": "double",
    "DoubleToken": "double",
    "DurationToken": "Duration",
    "FontWeightToken": "FontWeight",
    "TextStyleToken": "TextStyle",
    "BoxShadowToken": "BoxShadow",
}


def handle_for(name: str) -> str:
    """Derive a valid Dart handle from a registry token name."""
    parts = name.split(".")[1:]
    out = parts[0]
    for part in parts[1:]:
        if not part:
            continue
        if part[0].isdigit():
            out += part
        else:
            out += part[0].upper() + part[1:]
    if not IDENT_RE.match(out):
        raise SystemExit(f"generated handle is not a valid Dart identifier: {out!r} from {name!r}")
    return out


def fmt_double(value) -> str:
    f = float(value)
    s = f"{f:.4f}".rstrip("0").rstrip(".")
    return s if "." in s else s + ".0"


def fmt_color(value: str) -> str:
    if not COLOR_RE.match(value):
        raise SystemExit(f"invalid color token value: {value!r}")
    return f"0x{value[2:]}"


def render_value(token_name: str, entry: dict) -> str:
    typ = entry["type"]
    if typ == "color":
        return f"const Color({fmt_color(entry['light'])})"
    if typ == "radius":
        return f"const Radius.circular({fmt_double(entry['value'])})"
    if typ in ("space", "double"):
        return fmt_double(entry["value"])
    if typ == "duration":
        return f"const Duration(milliseconds: {int(entry['value'])})"
    if typ == "fontweight":
        weight = int(entry["value"])
        return f"FontWeight.w{weight}"
    if typ == "textstyle":
        fs = fmt_double(entry["value"]["fontSize"])
        lh = fmt_double(entry["value"]["lineHeight"])
        height = round(float(entry["value"]["lineHeight"]) / float(entry["value"]["fontSize"]), 4)
        family = entry["value"].get("fontFamily")
        fam = f", fontFamily: '{family}'" if family else ""
        return f"const TextStyle(fontSize: {fs}, height: {fmt_double(height)}{fam})"
    raise SystemExit(f"no inline renderer for type {typ!r}")


def render_shadow_list(shadows: list) -> str:
    parts = []
    for s in shadows:
        parts.append(
            "BoxShadow("
            f"color: Color({fmt_color(s['color'])}), "
            f"offset: Offset({fmt_double(s['offsetX'])}, {fmt_double(s['offsetY'])}), "
            f"blurRadius: {fmt_double(s['blur'])}, "
            f"spreadRadius: {fmt_double(s['spread'])})"
        )
    body = ",\n      ".join(parts)
    return f"[\n      {body},\n    ]"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    args = parser.parse_args()

    snapshot = json.loads(SNAPSHOT.read_text())
    tokens = snapshot["tokens"]
    source = snapshot["source"]

    # Group tokens by type, preserving deterministic (sorted) order.
    by_type: dict[str, list[tuple[str, dict]]] = {}
    for name in sorted(tokens):
        typ = tokens[name]["type"]
        by_type.setdefault(typ, []).append((name, tokens[name]))

    handles: list[str] = []
    seen_handles: dict[str, str] = {}
    for typ in sorted(by_type):
        for name, entry in by_type[typ]:
            handle = handle_for(name)
            if handle in seen_handles:
                raise SystemExit(
                    f"token handle collision: {name!r} and {seen_handles[handle]!r} "
                    f"both generate '{handle}' — rename one registry entry"
                )
            seen_handles[handle] = name
            handles.append((name, handle, entry))

    lines: list[str] = []
    w = lines.append

    # -- provenance header ---------------------------------------------------
    w("// GENERATED FILE — DO NOT EDIT BY HAND.")
    w("//")
    w(f"// Source     : {source['name']}@{source['version']} (npm registry)")
    w(f"// Registry   : {source['registryUrl']}")
    w(f"// Retrieved  : {source['retrievedAt']}")
    w("// Pinned CSS : tool/sources/*.css (see tool/sources/SHA256SUMS)")
    w("//")
    w("// Regenerate : python3 tool/normalize_tokens.py && python3 tool/generate_tokens.py")
    w("// Verify     : python3 tool/verify_generated.py")
    w("//")
    w("// SPDX-License-Identifier: MIT")
    w("//")
    w("// This file is generated from the committed normalized snapshot")
    w("// (tool/tokens/hero-tokens.normalized.json); regeneration is byte-identical.")
    w("//")
    w("// @dart=3.5")
    w("")
    w("library;")
    w("")
    w("import 'package:flutter/animation.dart'; // Cubic (not exported by painting)")
    w("import 'package:flutter/painting.dart';")
    w("import 'package:mix/mix.dart';")
    w("")

    # -- token handles ---------------------------------------------------------
    w("/// HeroUI v3 semantic token handles.")
    w("///")
    w("/// Call a token inside a styler chain to use its value as a Mix")
    w("/// reference (`HeroTokens.accent()`); resolve it with a BuildContext")
    w("/// to get the concrete value (`HeroTokens.accent.resolve(context)`).")
    w("/// Values live in the `HeroScope` token map (`buildHeroTokenMap`).")
    w("abstract final class HeroTokens {")
    w("  const HeroTokens._();")
    w("")
    for name, handle, entry in handles:
        typ = entry["type"]
        cls = TYPE_CLASS[typ]
        w(f"  /// `{name}` ({'shared' if 'value' in entry else 'per-theme'}).")
        w(f"  static const {cls} {handle} = {cls}('{name}');")
        w("")
    w("}")
    w("")

    # -- resolved value maps ---------------------------------------------------
    def emit_map(typ: str, map_name: str, picker):
        entries = by_type.get(typ, [])
        if not entries:
            return
        cls = TYPE_CLASS[typ]
        value_type = VALUE_IMPORTS[cls]
        w(f"/// Resolved `{typ}` token values.")
        w(f"final Map<{cls}, {value_type}> {map_name} = {{")
        for name, entry in entries:
            handle = handle_for(name)
            w(f"  HeroTokens.{handle}: {picker(entry)},")
        w("};")
        w("")

    # Colors per theme
    for theme in ("light", "dark"):
        emit_map("color", f"heroRoleColors{theme[0].upper() + theme[1:]}",
                 lambda e: f"const Color({fmt_color(e[theme])})")
    # Shared value maps
    emit_map("radius", "heroRadiusValues",
             lambda e: f"const Radius.circular({fmt_double(e['value'])})")
    emit_map("space", "heroSpaceValues", lambda e: fmt_double(e["value"]))
    emit_map("double", "heroDoubleValues", lambda e: fmt_double(e["value"]))
    emit_map("duration", "heroDurationValues",
             lambda e: f"const Duration(milliseconds: {int(e['value'])})")
    emit_map("fontweight", "heroFontWeightValues",
             lambda e: f"FontWeight.w{int(e['value'])}")
    emit_map("textstyle", "heroTextStyleValues", render_value_from_textstyle)

    # Shadows per theme
    for theme in ("light", "dark"):
        entries = by_type.get("boxshadow", [])
        if not entries:
            continue
        w(f"/// Resolved box-shadow token values ({theme}).")
        w(f"final Map<BoxShadowToken, List<BoxShadow>> heroShadowValues{theme[0].upper() + theme[1:]} = {{")
        for name, entry in entries:
            handle = handle_for(name)
            w(f"  HeroTokens.{handle}: {render_shadow_list(entry[theme])},")
        w("};")
        w("")

    # -- easing curves ----------------------------------------------------------
    w("/// HeroUI v3 easing curves (shared-theme.css --ease-*).")
    for key, points in snapshot["easing"].items():
        if key == "source":
            continue
        x1, y1, x2, y2 = (fmt_double(p) for p in points)
        const_name = "heroEase" + key[0].upper() + key[1:]
        w(f"const Cubic {const_name} = Cubic({x1}, {y1}, {x2}, {y2});")
    w("")

    # -- plain duration constants (non-styler contexts) --------------------------
    w("/// HeroUI v3 durations in milliseconds for non-styler contexts")
    w("/// (styler chains should use the `HeroTokens.duration*` tokens instead).")
    for name, entry in by_type.get("duration", []):
        # hero.duration.transition.fast -> transitionFast
        rest = name.split(".")[2:]
        handle = rest[0] + "".join(p[:1].upper() + p[1:] for p in rest[1:])
        suffix = "" if handle.lower().endswith("ms") else "Ms"
        w(f"const int hero{handle[:1].upper() + handle[1:]}{suffix} = {int(entry['value'])};")
    for name, entry in by_type.get("double", []):
        # component transition durations are authored as doubles, e.g.
        # hero.double.button.transitionBackgroundMs -> heroButtonTransitionBackgroundMs
        if not name.endswith("Ms"):
            continue
        if int(entry["value"]) != entry["value"]:
            continue  # not an integral millisecond value
        rest = name.split(".")[2:]
        handle = rest[0] + "".join(p[:1].upper() + p[1:] for p in rest[1:])
        suffix = "" if handle.lower().endswith("ms") else "Ms"
        w(f"const int hero{handle[:1].upper() + handle[1:]}{suffix} = {int(entry['value'])};")

    # -- plain const doubles (eager-computation contexts) ------------------------
    # Values consumed by non-Prop APIs (e.g. TransformStyleMixin.scale builds a
    # Matrix4 eagerly, so token refs would leak sentinel doubles) are emitted as
    # plain consts. Press scales are the only such values today.
    for name, entry in by_type.get("double", []):
        if "pressScale" not in name:
            continue
        rest = name.split(".")[2:]
        handle = rest[0] + "".join(p[:1].upper() + p[1:] for p in rest[1:])
        w(f"const double hero{handle[:1].upper() + handle[1:]} = {fmt_double(entry['value'])};")
    w("")

    # -- source manifest ----------------------------------------------------------
    inventory = snapshot["inventory"]
    w("/// Pinned source metadata for the generated tokens — what token tests")
    w("/// assert against (version, file hashes, inventory counts).")
    w("abstract final class HeroSourceManifest {")
    w("  const HeroSourceManifest._();")
    w(f"  static const String name = '{source['name']}';")
    w(f"  static const String version = '{source['version']}';")
    w(f"  static const String registryUrl = '{source['registryUrl']}';")
    w(f"  static const String retrievedAt = '{source['retrievedAt']}';")
    w("  static const Map<String, String> sourceFiles = {")
    for fname in sorted(source["files"]):
        w(f"    '{fname}': '{source['files'][fname]}',")
    w("  };")
    w("  // Inventory counts (generated from the normalized snapshot).")
    for typ in sorted(inventory):
        w(f"  static const int {typ}TokenCount = {inventory[typ]};")
    w(f"  static const int totalTokenCount = {sum(inventory.values())};")
    # Font family rides on the type-scale tokens (authored as `font.family`).
    for name, entry in by_type.get("textstyle", []):
        family = entry["value"].get("fontFamily")
        if family:
            w(f"  static const String fontFamily = '{family}';")
            break
    w("}")
    w("")

    out_path = args.out / "hero_tokens.g.dart"
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines))
    try:
        shown = out_path.relative_to(TOOL.parent.parent)
    except ValueError:
        shown = out_path
    print(f"generate: {len(handles)} token handles -> {shown}")
    return 0


def render_value_from_textstyle(entry: dict) -> str:
    return render_value("textstyle", entry)


if __name__ == "__main__":
    raise SystemExit(main())
