import 'package:flutter/services.dart' show SystemUiOverlayStyle;
import 'package:flutter/widgets.dart' show BorderRadius, Brightness, Color, Radius;
import 'package:forui/forui.dart';

/// The one shared [FThemeData] pair (light + dark) every package in this repo
/// themes off of — color values below are pulled directly from Medusa's own
/// admin UI design tokens (`@medusajs/ui-preset`, `packages/design-system/
/// ui-preset/src/theme/tokens/colors.ts` in medusajs/medusa), not Forui's
/// generic neutral/zinc palette, since every component in this design system
/// (`CStatusBadge`, `CDataTable`, `CProgressTabs`, ...) is explicitly modeled
/// on Medusa's own admin dashboard components and should actually look like
/// them, not just structurally match. Each [FColors] field below is commented
/// with the exact Medusa CSS custom property it's mapped from, so a future
/// token refresh can be diffed field-by-field against the source instead of
/// re-deriving the mapping from scratch. Font family needs no changes to get
/// here — Forui's own default ([FTypography.defaultFontFamily]) is already
/// Inter, the same family Medusa's `FONT_FAMILY_SANS` uses.
///
/// Kept in one place so `app/` (the shell) and every plugin's `ui/` package
/// (e.g. `congelado_engine_ui`) always render with the exact same look,
/// instead of each picking its own theme independently.
abstract final class CongeladoTheme {
  static FThemeData get light =>
      FThemeData(colors: _lightColors, touch: false, style: _styleFor(_lightColors));
  static FThemeData get dark =>
      FThemeData(colors: _darkColors, touch: false, style: _styleFor(_darkColors));

  // Forui's default FBorderRadius.md is 10px; most Forui widgets (buttons included) draw their
  // default rounding from that one tier. Medusa's own Button/Input use Tailwind's unmodified
  // `rounded-md` (0.375rem = 6px) — overriding just this tier, rather than every individual
  // widget's own style, is the one change that cascades the match across the whole app at once.
  // Built off FStyle.inherit() (same derivation FThemeData itself falls back to when no `style:`
  // is given) rather than FStyle's plain constructor, since that one requires supplying every
  // other sub-style (focusedOutlineStyle, iconStyle, sizes, tappableStyle, ...) by hand just to
  // change one field — copyWith() on the inherited default only touches what's actually different.
  static FStyle _styleFor(FColors colors) => FStyle.inherit(
    colors: colors,
    typography: FTypography.inherit(colors: colors, touch: false),
    touch: false,
  ).copyWith(borderRadius: const FBorderRadius(md: BorderRadius.all(Radius.circular(6))));

  static final FColors _lightColors = FColors(
    brightness: Brightness.light,
    systemOverlayStyle: SystemUiOverlayStyle.dark,
    barrier: const Color.fromRGBO(24, 24, 27, 0.4), // --bg-overlay
    background: const Color.fromRGBO(255, 255, 255, 1), // --bg-base
    foreground: const Color.fromRGBO(24, 24, 27, 1), // --fg-base
    primary: const Color.fromRGBO(39, 39, 42, 1), // --button-inverted
    primaryForeground: const Color.fromRGBO(255, 255, 255, 1), // --fg-on-inverted
    secondary: const Color.fromRGBO(250, 250, 250, 1), // --bg-component
    secondaryForeground: const Color.fromRGBO(24, 24, 27, 1), // --fg-base
    muted: const Color.fromRGBO(250, 250, 250, 1), // --bg-subtle
    mutedForeground: const Color.fromRGBO(113, 113, 122, 1), // --fg-muted
    destructive: const Color.fromRGBO(225, 29, 72, 1), // --button-danger
    destructiveForeground: const Color.fromRGBO(255, 255, 255, 1), // --fg-on-color
    error: const Color.fromRGBO(225, 29, 72, 1), // --fg-error
    errorForeground: const Color.fromRGBO(255, 255, 255, 1), // --fg-on-color
    card: const Color.fromRGBO(255, 255, 255, 1), // --bg-base
    border: const Color.fromRGBO(228, 228, 231, 1), // --border-base
  );

  static final FColors _darkColors = FColors(
    brightness: Brightness.dark,
    systemOverlayStyle: SystemUiOverlayStyle.light,
    barrier: const Color.fromRGBO(24, 24, 27, 0.72), // --bg-overlay
    background: const Color.fromRGBO(33, 33, 36, 1), // --bg-base
    foreground: const Color.fromRGBO(244, 244, 245, 1), // --fg-base
    primary: const Color.fromRGBO(82, 82, 91, 1), // --button-inverted
    primaryForeground: const Color.fromRGBO(24, 24, 27, 1), // --fg-on-inverted
    secondary: const Color.fromRGBO(39, 39, 42, 1), // --bg-component
    secondaryForeground: const Color.fromRGBO(244, 244, 245, 1), // --fg-base
    muted: const Color.fromRGBO(24, 24, 27, 1), // --bg-subtle
    mutedForeground: const Color.fromRGBO(113, 113, 122, 1), // --fg-muted
    destructive: const Color.fromRGBO(159, 18, 57, 1), // --button-danger
    destructiveForeground: const Color.fromRGBO(255, 255, 255, 1), // --fg-on-color
    error: const Color.fromRGBO(251, 113, 133, 1), // --fg-error
    errorForeground: const Color.fromRGBO(255, 255, 255, 1), // --fg-on-color
    card: const Color.fromRGBO(33, 33, 36, 1), // --bg-base
    border: const Color.fromRGBO(255, 255, 255, 0.08), // --border-base
  );
}
