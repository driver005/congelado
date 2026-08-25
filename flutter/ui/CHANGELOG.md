# Changelog

## 0.3.0 (in progress)

- **Widgetbook merged into this package**: the former `widgetbook/` sub-project
  (`congelado_hero_widgetbook`) is gone. Its app entrypoint is now
  `lib/main.dart`, the catalogue lives in `lib/src/widgetbook/`, and `web/`
  is this package's runner — `flutter run -d chrome` from `flutter/ui/` launches
  the catalogue directly. `widgetbook` is a regular dependency; the use-case
  scan paths in `tool/check_static.py` / `tool/check_parity.py` moved to
  `lib/src/widgetbook/use_cases`.
- Full HeroUI v3 parity: every top-level component of the pinned
  `@heroui/react@3.2.4` (84) now has a `Hero*` implementation in kowalski
  (remix+mix only), a spec-derived worksheet (`specs/components/*.yaml`), and
  a Widgetbook entry mirroring the HeroUI storybook. `tool/check_parity.py`
  pins the React index and asserts implementation + worksheet + use case per
  component (exit 0 == full parity); `tool/check_static.py` audits forbidden
  colors, token references and bracket balance.
- Static/typographic batch: HeroLabel, HeroDescription, HeroHeader, HeroLink,
  HeroKbd, HeroForm, HeroFieldset, HeroFieldError, HeroErrorMessage, HeroTag
  (+ Group), HeroTypography, HeroSurface, HeroScrollShadow, HeroCloseButton,
  HeroToolbar.
- Feedback batch: HeroAlert, HeroAlertDialog (+ `showHeroAlertDialog`),
  HeroToast (+ `showHeroToast`), HeroProgressCircle, HeroMeter,
  HeroEmptyState.
- Disclosure/navigation batch: HeroAccordion (+ items), HeroDisclosure
  (+ Group), HeroDrawer (+ `showHeroDrawer`), HeroDropdown
  (+ `showHeroDropdown`), HeroPopover (+ `showHeroPopover`), HeroBreadcrumbs,
  HeroPagination, HeroButtonGroup.
- Forms/selection batch: HeroTextarea, HeroSearchField, HeroNumberField,
  HeroInputOtp, HeroSelect, HeroSlider, HeroToggleButton (+ Group),
  HeroListBox (+ Item/Section), HeroMenu (+ Item/Section), HeroAutocomplete,
  HeroComboBox, HeroTimeField, HeroInputGroup, HeroCheckboxGroup,
  HeroSwitchGroup (plus the shared `HeroPopup` overlay helper).
- Date/time batch: HeroDateField, HeroDateInputGroup, HeroDatePicker
  (+ `showHeroDatePicker`), HeroDateRangePicker (+ `showHeroDateRangePicker`),
  HeroRangeCalendar, HeroCalendar, HeroCalendarYearPicker.
- Color batch: HeroColorArea, HeroColorField, HeroColorInputGroup,
  HeroColorPicker (+ `showHeroColorPicker`), HeroColorSlider, HeroColorSwatch,
  HeroColorSwatchPicker.
- Data batch: HeroTable.
- `heroEaseOutQuart` added to HeroMotion (`--ease-out-quart`).
- Fixed dialog-close crash: overlay actions/close triggers now pop with a
  dialog-bound context (Builder-wrapped in use cases; `showHeroAlertDialog`
  pops its own close trigger) instead of the caller's context, which can go
  stale when the host tree rebuilds (`Navigator.of` → "Unexpected null
  value").
- `HeroTooltip` rewritten: full placement support (top/right/bottom/left/
  start/end, react-aria semantics), HeroUI offsets (3, or 7 with arrow), the
  HeroUI 12×12 chevron arrow (fill overlay, stroke border/40, rotated per
  placement). Presentation matches the previous design — instant show/hide
  (no fade/zoom animation), 300ms open / 1500ms close defaults, no cursor
  override. The source `--tooltip-delay`/`--tooltip-close-delay` (1500/500)
  remain available via `delay`/`closeDelay`. Previously it was hard-wired to
  top via RemixTooltip with inverted delays.
- Overlay-entry lifecycle fixes: `HeroPopup.close()` now rebuilds its overlay
  entry (`OverlayEntry.markNeedsBuild`) so the exit animation actually runs
  and the entry is removed — previously the popup stayed stuck open (same
  stale-entry class of bug the tooltip had). All `OverlayEntry` removals
  (popup, tooltip, toast) are now guarded against same-frame double removes
  ("An OverlayEntry should be removed only once" assertion).
- Tooltip/popup overlay entries now capture the caller's `MixScope` and
  re-wrap the entry content in it — the navigator Overlay sits above the
  use case's HeroScope (e.g. a theme addon), so tokens resolved in the
  entry's own context painted the tooltip/popup with the wrong theme. This
  restores the original behavior (style resolved at the trigger) where the
  tooltip always matches the surrounding theme.
- Fixed "tooltip fills the screen": the Overlay lays entries out with
  `BoxConstraints.tight(screen size)`, so the tooltip panel (and popup
  panels) stretched to the full screen. Overlay content is now wrapped in
  `UnconstrainedBox` so it sizes to its content (max-w-xs 320) — the tooltip
  is a small bubble again, and popups are content-sized instead of
  full-height.
- Tooltip/popup positioning rewritten: `CompositedTransformFollower` +
  `UnconstrainedBox` centered the panel inside a full-screen box (tooltip
  appeared half a page from the trigger). Both now use a
  `CustomSingleChildLayout` driven by the trigger's measured global rect —
  exact placement on the requested side, content-sized, and popup outside
  taps now dismiss properly (full-screen barrier).
- Fixed overlay-anchor coordinates: the nearest `Overlay` can be a NESTED one
  (widgetbook's DeviceFrameAddon wraps every use case in its own Navigator
  inside the device frame), whose origin/scale differs from global
  coordinates — the anchor rect is now measured in the overlay's own
  coordinate space (`RenderBox.globalToLocal`), so top/right/bottom/left/
  start/end all land exactly on the trigger. Positions are also clamped into
  the visible bounds (8px margin) so a tooltip/popup with no room on the
  requested side stays fully on screen.
- `HeroDatePicker` / `HeroDateRangePicker` now open the calendar in an
  **anchored popover** below the trigger field (via the new shared
  `showHeroAnchoredOverlay` helper: overlay-local anchor rect, clamped
  on-screen, dismissed by outside tap) instead of a centered modal dialog —
  matching the HeroUI `.date-picker__popover`. `HeroDateInputGroup` gained a
  `fieldKey` to measure the tap target; the range picker's controlled-range
  state wrapper is reused inside the popover.
- Overlay theme + style hardening across tooltips/toasts/popovers:
  `showHeroAnchoredOverlay` now captures the caller's `MixScope` and re-wraps
  the entry content in it (the popover's calendar previously resolved tokens
  from the entry's own context, painting the selected day grey / normal days
  invisible when the entry scope differed from the trigger's theme).
  Overlay text no longer inherits the host's `DefaultTextStyle` decoration
  (tooltip panel, toast body, popover panel all reset `decoration: none`),
  and `HeroSpinner`'s `current` color now inherits the ambient text color
  instead of falling back to `Theme.colorScheme.primary` (grey circle).
- `HeroTooltip` visibility: no lingering — the tooltip shows while the
  trigger is hovered/focused and hides as soon as the pointer leaves
  (closeDelay default 0; instant show/hide, no animation).

## 0.2.0

- Component layer moved to `lib/src/kowalski/` (unchanged visually, deps
  remix+mix only). The skipper layer (go_router/riverpod/responsive/lucide/
  fl_chart/data_table_2/prefs/form_builder/cached_image) was removed again
  by decision — kowalski stays the single UI layer.

## 0.1.3

- Global accent-consistency: ALL selected/emphasis states use the accent
  family — tabs secondary (accent text + 2px underline), tabs primary
  (accent-soft pill + accent text); switch/checkbox/radio/progress/button
  primary already accent.
- Single source of truth for ALL component colors: new `HeroColor` role enum
  + `heroColorTokens()` in `foundation/hero_color_roles.dart`; Chip, Badge,
  Avatar, Progress and Spinner now resolve their per-color token pairs
  through this one table (previously each had its own duplicated switch).
  Audited every component for hardcoded design colors — all states
  token-driven (structural exemptions only: transparent fills, switch thumb
  `bg-white`, skeleton sweep per CSS).


- Exact-HeroUI style pass:
  - Automated component-CSS extraction (`tool/extract_components.py` +
    `tool/verify_components.py`): geometry/colors/states now generated from
    the pinned `@heroui/styles` component CSS, not hand-transcription.
  - `HeroFocusRing` wrapper: HeroUI `ring-2 ring-offset-2` outer focus ring
    (2px accent, 2px gap, 150ms) on Button/Input/Checkbox/Radio; recipe-level
    foregroundDecoration rings removed.
  - No Material ink ripple anywhere: `enableFeedback: false` on
    Switch/Checkbox/Radio (Button already done).
  - Typography: chip label `leading-5` (20px), button `text-base` lg leading
    (24px), Inter applied via generated type tokens + app/widgetbook themes.
  - Centered button/tab labels (`justify-center`), like HeroUI.
  - Golden baseline tests (`test/hero_golden_test.dart`, light+dark per
    component) — the "looks like HeroUI" gate; generate with
    `flutter test --update-goldens`.

## 0.1.2

- Fixed relative imports in `lib/src/components/*` (`../tokens/…` /
  `../foundation/…` instead of `../../…`) — the package now compiles.
- Fixed a generated-token handle collision (`doubleSpinnerStrokeWidth` was
  emitted twice); the generator now fails loudly on any handle collision.
- Generated `hero_tokens.g.dart` imports `package:flutter/animation.dart` for
  `Cubic` (it is not exported by painting or dart:ui).
- Scaffolded `web/` platform folders (`app/web/`, `app/ui/widgetbook/web/`) so
  `flutter run -d chrome` works out of the box.
- Fixed analyzer errors surfaced by a first real compile:
  - `registry.dart`: the showcase contribution uses a `WidgetBuilder` lambda
    instead of a constructor tear-off.
  - `hero_badge.dart` / `hero_chip.dart`: named-record destructuring
    (`final (:fill, :foreground) = …`), and chip font sizes are resolved from
    the tokens before use.
  - `hero_checkbox.dart` / `hero_radio.dart`: adapt `bool`/`T` callbacks to the
    nullable `bool?`/`T?` signatures of `RemixCheckbox`/`RemixRadioGroup`.
  - `hero_skeleton.dart`: `BorderRadius.all(radiusToken.resolve(context))`
    (the token resolves to a `Radius`, not a `double`).
  - `hero_tabs.dart`: hover color is `colorMuted().withOpacity(0.7)` (on the
    color ref), not `withOpacity` on the `TextStyler`.
  - `hero_badge.dart`: badge label typography applied via `RemixBadgeStyle.text`
    (`text-xs` / `font-medium`), fixing an unused-variable lint as well.
  - `hero_skeleton.dart`: fixed a StackOverflow in the shimmer/pulse
    `AnimatedBuilder`s — the animation builders captured the reassigned local
    `child` variable (a builder containing itself). The static placeholder is
    now passed via the canonical `AnimatedBuilder(animation:, child:, builder:)`
    pattern, so the builder references only its own `child` parameter and can
    never embed itself.
  - `hero_skeleton.dart`: the shimmer overlay `Stack` is now wrapped in an
    explicitly sized `SizedBox` — `StackFit.expand` inside an unbounded-height
    parent (Wrap in a scrollable) previously produced "BoxConstraints forces
    an infinite height".

## 0.1.1

- Added `widgetbook/` — a Widgetbook catalogue showcasing every `Hero*`
  component plus the token foundation (colors, type, radius, shadows), with
  theme (light/dark), device-frame, text-scale, alignment and grid addons.

## 0.1.0

- Initial release: HeroUI v3 design-system mirror on Remix.
  - Token pipeline (`tool/`) pinned to `@heroui/styles@3.2.4`:
    extract → normalize → generate → verify, byte-identical regeneration.
  - `HeroTokens` (237 generated token handles), `HeroScope`, `HeroTheme`.
  - Components: `HeroButton`, `HeroCard`, `HeroInput`, `HeroChip`,
    `HeroBadge`, `HeroTabs`, `HeroSkeleton`, `HeroTooltip`, `HeroModal`,
    `HeroProgress`, `HeroSpinner`, `HeroSwitch`, `HeroCheckbox`,
    `HeroRadioGroup`/`HeroRadio`, `HeroDivider`, `HeroAvatar`.
  - Worksheets for every component under `specs/components/`.
