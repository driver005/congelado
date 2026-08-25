import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// HeroUI v3 button color/variant classes (button.css `.button--*`).
///
/// v3 collapses v2's "variant x color" matrix into a single axis of color
/// classes; each entry below maps to its token pair (fill / foreground /
/// hover) plus any structural modifier (e.g. `outline` adds a 1px border).
enum HeroButtonVariant {
  /// Solid accent fill — the primary action (`.button--primary`).
  primary,

  /// Default fill with accent-soft foreground (`.button--secondary`).
  secondary,

  /// Default fill with default foreground (`.button--tertiary`).
  tertiary,

  /// Transparent with default fill on hover (`.button--ghost`).
  ghost,

  /// Transparent with a 1px border (`.button--outline`).
  outline,

  /// Solid danger fill (`.button--danger`).
  danger,

  /// Soft danger fill (`.button--danger-soft`).
  dangerSoft,
}

/// HeroUI v3 button sizes — desktop (`md:`) values from button.css:
/// `--sm` h-8 (32), default h-9 (36), `--lg` h-10 (40).
enum HeroButtonSize { sm, md, lg }

/// Token triple describing one [HeroButtonVariant]'s colors.
class _HeroButtonKind {
  const _HeroButtonKind({
    required this.fill,
    required this.foreground,
    required this.hover,
    this.outlined = false,
  });

  final ColorToken? fill;
  final ColorToken foreground;
  final ColorToken hover;

  /// Outline variant: transparent fill + 1px border instead of a fill.
  final bool outlined;

}

const _HeroButtonKinds = <HeroButtonVariant, _HeroButtonKind>{
  HeroButtonVariant.primary: _HeroButtonKind(
    fill: HeroTokens.colorAccent,
    foreground: HeroTokens.colorAccentForeground,
    hover: HeroTokens.colorAccentHover,
  ),
  HeroButtonVariant.secondary: _HeroButtonKind(
    fill: HeroTokens.colorDefault,
    foreground: HeroTokens.colorAccentSoftForeground,
    hover: HeroTokens.colorDefaultHover,
  ),
  HeroButtonVariant.tertiary: _HeroButtonKind(
    fill: HeroTokens.colorDefault,
    foreground: HeroTokens.colorForeground,
    hover: HeroTokens.colorDefaultHover,
  ),
  HeroButtonVariant.ghost: _HeroButtonKind(
    fill: null, // transparent fill (colorTransparent token)
    foreground: HeroTokens.colorDefaultForeground,
    hover: HeroTokens.colorDefault,
  ),
  HeroButtonVariant.outline: _HeroButtonKind(
    fill: null, // transparent fill (colorTransparent token)
    foreground: HeroTokens.colorDefaultForeground,
    hover: HeroTokens.colorOutlineHover,
    outlined: true,
  ),
  HeroButtonVariant.danger: _HeroButtonKind(
    fill: HeroTokens.colorDanger,
    foreground: HeroTokens.colorDangerForeground,
    hover: HeroTokens.colorDangerHover,
  ),
  HeroButtonVariant.dangerSoft: _HeroButtonKind(
    fill: HeroTokens.colorDangerSoft,
    foreground: HeroTokens.colorDangerSoftForeground,
    hover: HeroTokens.colorDangerSoftHover,
  ),
};

/// Styler recipes are pure functions of a few enums — memoize them. Stylers
/// are immutable value objects, so this is safe and cheap.
final Map<(HeroButtonVariant, HeroButtonSize, bool), RemixButtonStyle>
    _heroButtonStyleCache = {};

/// Returns the [RemixButtonStyle] for a HeroUI v3 button.
///
/// Mirror of button.css:
/// * geometry — fixed height + horizontal padding + 24px radius (`rounded-3xl`),
///   `gap-2`, `text-sm`/`text-base` `font-medium`;
/// * states — hover/pressed swap to the kind's hover fill; focus paints a 2px
///   accent ring via `foregroundDecoration` (no layout shift, base border
///   intact — see the design-system playbook);
/// * loading — `RemixButton` folds `loading` into the disabled widget-state,
///   so `heroButtonStyle(loading: true)` keeps the kind's colors (the facade
///   shows the spinner) instead of applying a gray disabled treatment;
/// * disabled — HeroUI fades the whole button to `--disabled-opacity` (0.5);
///   the facade applies that as an `Opacity` so the recipe stays kind-colored.
RemixButtonStyle heroButtonStyle({
  HeroButtonVariant variant = HeroButtonVariant.primary,
  HeroButtonSize size = HeroButtonSize.md,
  bool loading = false,
}) {
  return _heroButtonStyleCache.putIfAbsent((variant, size, loading), () {
    final kind = _HeroButtonKinds[variant]!;
    final base = _heroButtonBaseStyle(size: size, outlined: kind.outlined);

    var style = base
        .color(kind.fill?.call() ?? HeroTokens.colorTransparent())
        .labelColor(kind.foreground())
        .iconColor(kind.foreground())
        .spinner(RemixSpinnerStyle().indicatorColor(kind.foreground()))
        // HeroUI: --button-bg-pressed: var(--button-bg-hover), plus the
        // :active transform scale (button.css .button:active).
        .onHovered(RemixButtonStyle().color(kind.hover()))
        .onPressed(
          RemixButtonStyle()
              .color(kind.hover())
              .scale(_heroButtonPressScale(size)),
        );

    // Focus ring is rendered by the HeroFocusRing wrapper in the facade
    // (HeroUI ring-2 ring-offset-2, outside the element) — not here.
    return style;
  });
}

/// HeroUI button color transitions run at 100ms on `--ease-out`; the press
/// transform at 250ms on `--ease-smooth`. Mix animates style changes, so a
/// single animation config reproduces the color cross-fade.
RemixButtonStyle _heroButtonBaseStyle({
  required HeroButtonSize size,
  required bool outlined,
}) {
  final (height, paddingX, fontSize, iconSize) = switch (size) {
    HeroButtonSize.sm => (
        HeroTokens.doubleButtonHeightSm(),
        HeroTokens.doubleButtonPaddingXSm(),
        HeroTokens.doubleButtonFontSizeSm(),
        HeroTokens.doubleButtonIconSizeSm(),
      ),
    HeroButtonSize.md => (
        HeroTokens.doubleButtonHeightMd(),
        HeroTokens.doubleButtonPaddingXMd(),
        HeroTokens.doubleButtonFontSizeMd(),
        HeroTokens.doubleButtonIconSizeMd(),
      ),
    HeroButtonSize.lg => (
        HeroTokens.doubleButtonHeightLg(),
        HeroTokens.doubleButtonPaddingXLg(),
        HeroTokens.doubleButtonFontSizeLg(),
        HeroTokens.doubleButtonIconSizeLg(),
      ),
  };

  var style = RemixButtonStyle(
    animation: AnimationConfig.curve(
      duration: const Duration(milliseconds: heroButtonTransitionBackgroundMs),
      curve: heroEaseOut,
    ),
  )
      .height(height)
      .paddingX(paddingX)
      // HeroUI: `.button` is `items-center justify-center`.
      .alignment(Alignment.center)
      .spacing(HeroTokens.space2())
      .borderRadiusAll(HeroTokens.radius3xl())
      .label(
        TextStyler()
            .style(HeroTokens.typeSm.mix())
            .fontSize(fontSize)
            // text-sm -> leading 20px; text-base (lg) -> leading 24px.
            .height(size == HeroButtonSize.lg ? 24.0 / 16.0 : 20.0 / 14.0)
            .fontWeight(HeroTokens.weightMedium()),
      )
      .iconSize(iconSize)
      .spinner(RemixSpinnerStyle(size: iconSize));

  if (outlined) {
    style = style.borderAll(
      color: HeroTokens.colorBorder(),
      width: HeroTokens.doubleBorderWidth(),
    );
  }

  return style;
}

double _heroButtonPressScale(HeroButtonSize size) => switch (size) {
      HeroButtonSize.sm => heroButtonPressScaleSm,
      HeroButtonSize.md => heroButtonPressScaleMd,
      HeroButtonSize.lg => heroButtonPressScaleLg,
    };

/// A HeroUI v3 button.
///
/// ```dart
/// HeroButton(
///   label: 'Continue',
///   variant: HeroButtonVariant.primary,
///   onPressed: () {},
/// )
/// ```
class HeroButton extends StatelessWidget {
  const HeroButton({
    super.key,
    required this.label,
    this.variant = HeroButtonVariant.primary,
    this.size = HeroButtonSize.md,
    this.onPressed,
    this.loading = false,
    this.enabled = true,
    this.leadingIcon,
    this.trailingIcon,
    this.focusNode,
  });

  /// The button label.
  final String label;

  /// Color class — see [HeroButtonVariant].
  final HeroButtonVariant variant;

  /// Size — see [HeroButtonSize].
  final HeroButtonSize size;

  /// Callback when the button is pressed. When null the button is disabled.
  final VoidCallback? onPressed;

  /// Shows the loading spinner and keeps the kind's colors (no gray-out).
  final bool loading;

  /// Whether the button is enabled.
  final bool enabled;

  /// Optional leading icon (HeroUI buttons show icons inline, `gap-2`).
  final IconData? leadingIcon;

  /// Optional trailing icon.
  final IconData? trailingIcon;

  final FocusNode? focusNode;

  @override
  Widget build(BuildContext context) {
    final disabled = !enabled || onPressed == null;
    final radius = HeroTokens.radius3xl.resolve(context).x;
    return HeroFocusRing(
      radius: radius,
      builder: (context, node, focused) => Opacity(
        // HeroUI disabled state: `opacity: var(--disabled-opacity)` (0.5).
        opacity: disabled && !loading
            ? HeroTokens.doubleDisabledOpacity.resolve(context)
            : 1.0,
        child: RemixButton(
          style: heroButtonStyle(variant: variant, size: size, loading: loading),
          label: label,
          leadingIcon: leadingIcon,
          trailingIcon: trailingIcon,
          loading: loading,
          enabled: enabled,
          onPressed: onPressed,
          focusNode: node,
          // HeroUI buttons are flat — no Material ink ripple.
          enableFeedback: false,
        ),
      ),
    );
  }
}
