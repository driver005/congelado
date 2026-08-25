# ADR 0001 — HeroUI v3 design-system mirror on Remix

Status: accepted
Date: 2026-03-10

## Context

The congelado Flutter shell (`app/`) and its plugin UIs share one Forui-based
design system (`CongeladoTheme` in `sdk/ui_dart`). We want (a) the Remix
component library in the project and (b) a design system on top of it that
mirrors HeroUI v3 (`heroui.com`), whose web (React) and native (React Native)
docs share the same token model.

## Decisions

### 1. Package: `flutter/ui` (`congelado_hero_ui`)

A standalone design-system package built on Remix, following the
`building-remix-design-system` methodology (extract → normalize → generate →
verify token pipeline; `HeroScope`; `Hero*` components). It lives at
`flutter/ui/` (moved from `sdk/ui_hero` by team decision) as a separate package
with its own token scope, because Mix tokens are resolved against a single
`MixScope` per subtree and mixing Forui's theme with HeroUI tokens in one
package would create a dependency tangle. The `Widgetbook` catalogue is the package's own
app entrypoint (`lib/main.dart`, see `flutter/ui/`).

The existing Forui-based theme stays untouched: the shell chrome keeps
`CongeladoTheme`, and the HeroUI mirror is layered on top (`HeroScope` in
`MaterialApp.builder`, showcase page registered in `registry.dart`).

### 2. Source pinning (tier classification)

| Domain | Tier | Source |
| --- | --- | --- |
| Theme colors, shadows, radius base, spacing, durations, easing | 1 | `@heroui/styles@3.2.4` npm tarball: `themes/default/variables.css`, `themes/shared/theme.css` (sha256-pinned in `tool/sources/SHA256SUMS`) |
| Component anatomy (heights, paddings, per-component radii) | 2/3 | `@heroui/styles@3.2.4` `dist/components/*.css`, transcribed with citations into `tool/authored/hero-component-tokens.json` |
| Type scale | 3 | Tailwind CSS v4 defaults as used by the component CSS |
| Component machinery | — | `remix@0.2.0` + `mix@2.x` (pub.dev) |

Upgrade procedure: bump the pinned tarball, re-run extract → normalize →
generate, review the diffs (`tool/tokens/hero-tokens.normalized.json`,
`lib/src/tokens/generated/hero_tokens.g.dart`), update inventory assertions.

### 3. Token model

Preserve HeroUI v3's own model: **role-based semantic tokens** (accent,
default, success, warning, danger + surface/overlay/field roles), two themes
(light/dark), with hover/soft/foreground variants computed from the base via
CSS `color-mix(in oklab, …)`. No numbered scales are invented. Colors are
converted OKLCH → sRGB at extract time and shipped as `0xAARRGGBB`.

### 4. Components

Hand-written facades over Remix widgets + `hero*Style(...)` recipes
(`@MixWidget` codegen is not used: no Dart toolchain in this repo, and the
loading/error state collisions make several components hand-written facades
anyway). Every component has a worksheet under `specs/components/`.

### 5. Explicitly out of scope (first release)

- The HeroUI *v2* (NextUI) look — the docs at heroui.com are v3; we mirror v3.
- Sliding tab indicators (Remix 0.2.0 has no indicator slot) — selected tabs
  are styled directly; documented per worksheet.
- Font bundling (Inter is HeroUI's family but is not shipped; consumers add it).
- CI workflow file for `tool/verify_generated.py` — the repo's CI is Bazel-based
  and has no Dart/Python job today; `verify_generated.py` is CI-ready (pure
  stdlib, read-only) but not yet wired into a workflow.

## Consequences

- Token values are reproducible and diffable; drift from the pinned source
  fails `verify_generated.py`.
- `Hero*` widgets require a `HeroScope` ancestor (like Fortal requires
  `FortalScope`); documented in `README.md` and `hero_scope.dart`.
- Consumers need Dart ≥ 3.11 / Flutter ≥ 3.38 (remix 0.2.0 floor); the app's
  `sdk:` floor was bumped to `^3.11.0` accordingly.
