#!/usr/bin/env python3
"""Normalize raw HeroUI tokens into the repository-owned token registry.

Stage 2 of the token pipeline (extract -> normalize -> generate -> verify).

Input : tool/build/raw-tokens.json            (from extract_tokens.py)
        tool/authored/hero-component-tokens.json
Output: tool/tokens/hero-tokens.normalized.json  (COMMITTED snapshot)

Every token in the registry carries:
  * a semantic name (namespace: hero.<domain>.<name>);
  * a type (color | radius | space | double | duration | fontweight |
    textstyle | boxshadow);
  * per-theme values where the source differentiates light/dark, otherwise a
    single shared value.

Determinism rules: stableStringify (deep-sorted keys), floats rounded to 4
decimals, colors emitted as 0xAARRGGBB. Missing dark values inherit light
(CSS custom-property inheritance: the .dark block only overrides a subset).
"""
from __future__ import annotations

import json
import re
from pathlib import Path

TOOL = Path(__file__).resolve().parent
RAW = TOOL / "build" / "raw-tokens.json"
AUTHORED = TOOL / "authored" / "hero-component-tokens.json"
OUT = TOOL / "tokens" / "hero-tokens.normalized.json"

# CSS variable name -> registry token name (light/dark per-theme colors)
COLOR_ROLES: dict[str, str] = {
    "background": "background",
    "foreground": "foreground",
    "surface": "surface",
    "surface-foreground": "surfaceForeground",
    "surface-hover": "surfaceHover",
    "surface-secondary": "surfaceSecondary",
    "surface-secondary-foreground": "surfaceSecondaryForeground",
    "surface-tertiary": "surfaceTertiary",
    "surface-tertiary-foreground": "surfaceTertiaryForeground",
    "overlay": "overlay",
    "overlay-foreground": "overlayForeground",
    "muted": "muted",
    "default": "default",
    "default-foreground": "defaultForeground",
    "accent": "accent",
    "accent-foreground": "accentForeground",
    "success": "success",
    "success-foreground": "successForeground",
    "warning": "warning",
    "warning-foreground": "warningForeground",
    "danger": "danger",
    "danger-foreground": "dangerForeground",
    "segment": "segment",
    "segment-foreground": "segmentForeground",
    "border": "border",
    "separator": "separator",
    "focus": "focus",
    "link": "link",
    "backdrop": "backdrop",
    "default-hover": "defaultHover",
    "accent-hover": "accentHover",
    "success-hover": "successHover",
    "warning-hover": "warningHover",
    "danger-hover": "dangerHover",
    "default-soft": "defaultSoft",
    "default-soft-foreground": "defaultSoftForeground",
    "default-soft-hover": "defaultSoftHover",
    "accent-soft": "accentSoft",
    "accent-soft-foreground": "accentSoftForeground",
    "accent-soft-hover": "accentSoftHover",
    "danger-soft": "dangerSoft",
    "danger-soft-foreground": "dangerSoftForeground",
    "danger-soft-hover": "dangerSoftHover",
    "warning-soft": "warningSoft",
    "warning-soft-foreground": "warningSoftForeground",
    "warning-soft-hover": "warningSoftHover",
    "success-soft": "successSoft",
    "success-soft-foreground": "successSoftForeground",
    "success-soft-hover": "successSoftHover",
    "background-secondary": "backgroundSecondary",
    "background-tertiary": "backgroundTertiary",
    "background-inverse": "backgroundInverse",
    "separator-secondary": "separatorSecondary",
    "separator-tertiary": "separatorTertiary",
    "border-secondary": "borderSecondary",
    "border-tertiary": "borderTertiary",
    "field-background": "field",
    "field-hover": "fieldHover",
    "field-focus": "fieldFocus",
    "field-foreground": "fieldForeground",
    "field-placeholder": "fieldPlaceholder",
    "field-border": "fieldBorder",
    "field-border-hover": "fieldBorderHover",
    "field-border-focus": "fieldBorderFocus",
    "white": "white",
    "black": "black",
    "snow": "snow",
    "eclipse": "eclipse",
}

COLOR_RE = re.compile(r"^0x[0-9A-F]{8}$")


def stable_stringify(obj) -> str:
    return json.dumps(obj, indent=2, sort_keys=True, ensure_ascii=False)


def round4(value: float) -> float:
    return round(float(value), 4)


def build_registry(raw: dict, authored: dict) -> dict:
    tokens: dict[str, dict] = {}

    def add(name: str, typ: str, light=None, dark=None, value=None):
        entry: dict = {"type": typ}
        if value is not None:
            entry["value"] = value
        else:
            entry["light"] = light
            entry["dark"] = dark
        tokens[name] = entry

    # -- colors --------------------------------------------------------------
    variables = raw["variables"]
    for css_name, token_name in COLOR_ROLES.items():
        var = variables.get(css_name)
        if var is None:
            raise SystemExit(f"missing CSS variable --{css_name} in extracted raw tokens")
        light = var.get("light")
        dark = var.get("dark")
        if light is None:
            raise SystemExit(f"--{css_name} has no light value")
        if not COLOR_RE.match(light):
            raise SystemExit(f"--{css_name} light value not a color: {light!r}")
        dark = dark if dark is not None else light  # custom-property inheritance
        add(f"hero.color.{token_name}", "color", light=light, dark=dark)

    # -- structural transparent (pipeline token so components never hardcode) ---
    add("hero.color.transparent", "color", light="0x00000000", dark="0x00000000")

    # -- derived colors (authored: color at fixed alpha over transparent) ------
    for name, spec in authored.get("derivedColors", {}).items():
        base = tokens[f"hero.color.{spec['base']}"]["light"]
        if not COLOR_RE.match(base):
            raise SystemExit(f"derived color base not a color: {base!r}")
        a = round(spec["alpha"] * 255)
        derived_light = f"0x{a:02X}" + base[4:]
        add(f"hero.color.{name}", "color", light=derived_light, dark=derived_light)

    # -- shadows -------------------------------------------------------------
    for css_name, token_name in (("surface-shadow", "surface"),
                                 ("overlay-shadow", "overlay"),
                                 ("field-shadow", "field")):
        var = variables[css_name]
        add(f"hero.shadow.{token_name}", "boxshadow",
            light=var["light"], dark=var["dark"])

    # -- radius scale (shared; from shared-theme.css, --radius: 0.5rem) ------
    radius_scale = {
        "xs": 2.0, "sm": 4.0, "md": 6.0, "lg": 8.0, "xl": 12.0,
        "2xl": 16.0, "3xl": 24.0, "4xl": 32.0, "field": 12.0,
    }
    for name, px in radius_scale.items():
        add(f"hero.radius.{name}", "radius", value=round4(px))

    # -- spacing scale (shared) ----------------------------------------------
    for key, px in authored["spacingScale"].items():
        if key in ("source",):
            continue
        handle = key.replace(".", "_")
        add(f"hero.space.{key}", "space", value=round4(px))

    # -- durations (shared) --------------------------------------------------
    durations = {
        "transition.fast": 100,
        "transition.base": 150,
        "transition.medium": 250,
        "transition.slow": 300,
        "skeleton": 2000,
        "spin": 750,
        "tooltip.delay": 1500,
        "tooltip.closeDelay": 500,
    }
    for name, ms in durations.items():
        add(f"hero.duration.{name}", "duration", value=int(ms))

    # -- font weights (shared) ------------------------------------------------
    add("hero.weight.medium", "fontweight", value=500)
    add("hero.weight.semibold", "fontweight", value=600)

    # -- type scale (shared) ---------------------------------------------------
    font_family = authored.get("font", {}).get("family")
    for name, spec in authored["typeScale"].items():
        if name == "source":
            continue
        value = {
            "fontSize": round4(spec["fontSize"]),
            "lineHeight": round4(spec["lineHeight"]),
        }
        if font_family:
            value["fontFamily"] = font_family
        add(f"hero.type.{name}", "textstyle", value=value)

    # -- global doubles (shared) -----------------------------------------------
    add("hero.double.borderWidth", "double", value=1.0)
    add("hero.double.focusRingWidth", "double", value=2.0)  # utilities: focus-ring ring-2
    add("hero.double.ringOffset", "double", value=2.0)      # utilities: --ring-offset-width
    add("hero.double.disabledOpacity", "double", value=0.5)

    # -- component metrics (authored, shared) -----------------------------------
    # Numeric "radius"-named metrics become RadiusTokens (RadiusRef is Prop-
    # based and resolves inside styler chains); everything else is a double.
    def flatten_component(prefix: str, group: dict):
        for key, value in group.items():
            if key == "source":
                continue
            is_radius = "radius" in key.lower()
            if isinstance(value, dict):
                # e.g. height: {sm, md, lg}
                for sub, v in value.items():
                    if sub == "source":
                        continue
                    if isinstance(v, (int, float)):
                        name = f"hero.{'radius' if is_radius else 'double'}.{prefix}.{key}.{sub}"
                        add(name, "radius" if is_radius else "double",
                            value=round4(v) if isinstance(v, float) else v)
            elif isinstance(value, (int, float)):
                name = f"hero.{'radius' if is_radius else 'double'}.{prefix}.{key}"
                add(name, "radius" if is_radius else "double",
                    value=round4(value) if isinstance(value, float) else value)
            # non-numeric scalars (e.g. fontWeight: "medium") ignored

    for component, group in authored["components"].items():
        flatten_component(component, group)

    return tokens


def main() -> int:
    raw = json.loads(RAW.read_text())
    authored = json.loads(AUTHORED.read_text())

    tokens = build_registry(raw, authored)

    inventory = {}
    for typ in ("color", "radius", "space", "double", "duration",
                "fontweight", "textstyle", "boxshadow"):
        inventory[typ] = sum(1 for t in tokens.values() if t["type"] == typ)

    snapshot = {
        "schemaVersion": 1,
        "source": {
            "name": authored["source"]["name"],
            "version": authored["source"]["version"],
            "registryUrl": authored["source"]["registryUrl"],
            "retrievedAt": authored["source"]["retrievedAt"],
            "files": authored["source"]["files"],
            "citationNote": authored["source"]["citationNote"],
        },
        "themes": raw["themes"],
        "easing": authored["easing"],
        "inventory": inventory,
        "tokens": tokens,
    }

    OUT.write_text(stable_stringify(snapshot) + "\n")
    total = sum(inventory.values())
    print(f"normalize: {total} tokens ({inventory}) -> {OUT.relative_to(TOOL.parent.parent)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
