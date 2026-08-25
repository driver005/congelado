# congelado_hero_ui

A HeroUI v3 (`heroui.com`) design-system mirror for Flutter, built on
[Remix](https://pub.dev/packages/remix) (headless UI behavior + the Mix
styling engine). It ships the exact HeroUI v3 default-theme tokens — colors,
radius scale, shadows, spacing, type scale, easing — generated from the pinned
`@heroui/styles@3.2.4` npm package, plus HeroUI-vocabulary components built as
Remix recipes.

> Mirror, not a port: every value is traced to `@heroui/styles` and the
> component CSS (see `tool/`), but this is an independent implementation and
> is not affiliated with HeroUI.

## Packages

| Layer | What it provides |
| --- | --- |
| `HeroScope` | Provides `HeroTheme.light` / `HeroTheme.dark` tokens to the subtree (a `MixScope`). **Required** above any `Hero*` widget. |
| `HeroTokens` | Static token handles (`ColorToken`, `RadiusToken`, `SpaceToken`, …) usable in styler chains (`HeroTokens.accent()`), resolved with `HeroTokens.accent.resolve(context)`. |
| `hero*Style(...)` | Per-component recipe functions returning Remix stylers (`RemixButtonStyle`, …). |
| `Hero*` widgets | Hand-written facades over the Remix widgets, in HeroUI vocabulary (variant/size/color enums). |

## Install

The package ships inside this repo at `flutter/ui`. A consumer package depends
on it with a relative path, e.g.:

```yaml
dependencies:
  congelado_hero_ui:
    path: ../flutter/ui
```

Other packages can depend on it with a relative path, e.g. `../flutter/ui`.

Requires the same consumer floor as `remix` 0.2.0: Dart `>=3.11.0`,
Flutter `>=3.38.1`.

## Usage

```dart
import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';

void main() => runApp(const App());

class App extends StatelessWidget {
  const App({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      builder: (context, child) => HeroScope(
        theme: MediaQuery.platformBrightnessOf(context) == Brightness.dark
            ? HeroTheme.dark
            : HeroTheme.light,
        child: child!,
      ),
      home: const Example(),
    );
  }
}

class Example extends StatelessWidget {
  const Example({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: HeroButton(
          label: 'Continue',
          variant: HeroButtonVariant.primary,
          onPressed: () {},
        ),
      ),
    );
  }
}
```

The recipes can also be used directly with Remix widgets:

```dart
RemixButton(
  label: 'Ghost',
  style: heroButtonStyle(variant: HeroButtonVariant.ghost),
  onPressed: () {},
)
```

## Component layers

### kowalski — the core design system (`lib/src/kowalski/`)
The HeroUI v3 mirror, dependency-light (remix + mix only):
`HeroButton`, `HeroCard`, `HeroInput`, `HeroChip`, `HeroBadge`, `HeroTabs`,
`HeroSkeleton`, `HeroTooltip`, `HeroModal` (+ `showHeroModal`),
`HeroProgress`, `HeroSpinner`, `HeroSwitch`, `HeroCheckbox`, `HeroRadioGroup`
(+ `HeroRadio`), `HeroDivider`, `HeroAvatar`, and the content/typographic set:
`HeroLabel`, `HeroDescription`, `HeroHeader`, `HeroLink`, `HeroKbd`,
`HeroForm`, `HeroFieldset`, `HeroFieldError`, `HeroErrorMessage`, `HeroTag`
(+ `HeroTagGroup`), `HeroTypography`, `HeroSurface`, `HeroScrollShadow`,
`HeroCloseButton`, `HeroToolbar`.


Every component ships a worksheet under `specs/components/` documenting its
anatomy, sizes, states, token consumption, and every approximation vs. the
pinned HeroUI CSS.

## HeroUI parity

`tool/check_parity.py` pins `@heroui/react@3.2.4` (npm tarball extracted in
`.reference/`) and asserts, for every top-level component, that a `Hero*`
implementation exists, a worksheet is committed, and a Widgetbook use case
mirrors the HeroUI storybook story:

```bash
cd flutter/ui
python3 tool/check_parity.py   # exit 0 == full parity
```

## Token pipeline

The token surface is generated — never hand-copied — from pinned sources:

```
@heroui/styles@3.2.4 (npm tarball, sha256-pinned in tool/sources/SHA256SUMS)
        │  tool/extract_tokens.py      (CSS vars → raw tokens; oklch, color-mix)
        ▼
tool/tokens/hero-tokens.normalized.json   (COMMITTED snapshot)
        │  tool/generate_tokens.py
        ▼
lib/src/tokens/generated/hero_tokens.g.dart  (COMMITTED)
        ▲  tool/verify_generated.py     (read-only, CI-able)
```

```bash
cd flutter/ui
python3 tool/verify_generated.py        # read-only check: all stages pass
python3 tool/extract_tokens.py && python3 tool/normalize_tokens.py \
  && python3 tool/generate_tokens.py    # full regeneration
```

Component measurements (heights, paddings, per-component radii) are a
tier-2/3 transcription of the component CSS, authored with citations in
`tool/authored/hero-component-tokens.json`.

## Widgetbook catalogue

This package is runnable as an app: `lib/main.dart` launches the
[Widgetbook](https://widgetbook.io) catalogue, showcasing every `Hero*`
component (variants, sizes, states) plus the token foundation (colors, type
scale, radius, shadows). The catalogue code lives in `lib/src/widgetbook/`
(app entrypoint + use cases); `web/` is the runner:

```bash
cd flutter/ui
flutter pub get
flutter run -d chrome
```

The Theme addon drives `HeroScope` (light/dark), and several use cases are
knob-driven. See `lib/src/widgetbook/hero_widgetbook_app.dart` to register
new components.

## Tests

```bash
cd flutter/ui
flutter test
```

The suite covers the token surface (inventory counts vs `HeroSourceManifest`,
spot values against the pinned `@heroui/styles@3.2.4` colors, cached and
unmodifiable base maps, overrides), the scope contract (`themeOf` /
`overridesOf` / notify-on-change), and per-component behavior (render + tap,
disabled/loading states, measured geometry, and a full light+dark build smoke
test of every `Hero*` component).

## License

BSD-3-Clause (see `LICENSE`). The design tokens are derived from
`@heroui/styles` (MIT) — see `NOTICE` for full upstream attribution.
