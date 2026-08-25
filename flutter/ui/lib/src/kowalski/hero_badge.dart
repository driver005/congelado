import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../foundation/hero_color_roles.dart';
import '../tokens/hero_tokens.dart';

/// HeroUI v3 badge color classes (badge.css `.badge--<color>`).
enum HeroBadgeColor { accent, default_, success, warning, danger }

/// HeroUI v3 badge variants (badge.css compound classes).
enum HeroBadgeVariant {
  /// Solid fill (`.badge--primary.badge--<color>`).
  solid,

  /// Default fill with colored text (`.badge--secondary`).
  secondary,

  /// Soft tinted fill (`.badge--<color>.badge--soft`).
  soft,
}

/// HeroUI v3 badge sizes (badge.css `.badge--sm/md/lg`).
enum HeroBadgeSize { sm, md, lg }

final Map<(HeroBadgeVariant, HeroBadgeColor, HeroBadgeSize), RemixBadgeStyle>
    _heroBadgeStyleCache = {};

/// Returns the [RemixBadgeStyle] for a HeroUI v3 badge.
///
/// badge.css: `.badge` — `min-h-7 min-w-7 rounded-3xl text-xs`, 1px border in
/// the background color (`background-clip: padding-box`); `--sm` (min 16,
/// `rounded-xl`, 10px), `--lg` (min 32, `rounded-2xl`, text-sm).
/// Uses explicit padding (like Fortal badge styles) for consistent text spacing.
RemixBadgeStyle heroBadgeStyle({
  HeroBadgeVariant variant = HeroBadgeVariant.secondary,
  HeroBadgeColor color = HeroBadgeColor.default_,
  HeroBadgeSize size = HeroBadgeSize.md,
}) {
  return _heroBadgeStyleCache.putIfAbsent((variant, color, size), () {
    final (:fill, :foreground) = _badgeColors(variant, color);
    final (paddingX, paddingY, radius, fontSize, minSize) = switch (size) {
      HeroBadgeSize.sm => (
          HeroTokens.doubleBadgePaddingXSm(),
          HeroTokens.doubleBadgePaddingYSm(),
          HeroTokens.radiusXl(),
          HeroTokens.doubleBadgeFontSizeSm(),
          HeroTokens.doubleBadgeMinSizeSm(),
        ),
      HeroBadgeSize.md => (
          HeroTokens.doubleBadgePaddingXMd(),
          HeroTokens.doubleBadgePaddingYMd(),
          HeroTokens.radius3xl(),
          HeroTokens.doubleBadgeFontSizeMd(),
          HeroTokens.doubleBadgeMinSizeMd(),
        ),
      HeroBadgeSize.lg => (
          HeroTokens.doubleBadgePaddingXLg(),
          HeroTokens.doubleBadgePaddingYLg(),
          HeroTokens.radius2xl(),
          HeroTokens.doubleBadgeFontSizeLg(),
          HeroTokens.doubleBadgeMinSizeLg(),
        ),
    };

    var style = RemixBadgeStyle()
        .color(fill?.call() ?? HeroTokens.colorTransparent())
        .foregroundColor(foreground())
        .borderRadius(BorderRadiusGeometryMix.all(radius))
        .alignment(Alignment.center)
        .padding(
          EdgeInsetsGeometryMix.symmetric(
            horizontal: paddingX,
            vertical: paddingY,
          ),
        )
        .text(
          TextStyler()
              .fontSize(fontSize)
              .fontWeight(HeroTokens.weightMedium()),
        )
        .constraints(
          BoxConstraintsMix(minWidth: minSize, minHeight: minSize),
        );

    // HeroUI badges draw a 1px border in the page background color
    // (`--badge-border: var(--background)`), which reads as a white ring
    // around solid badges — badge.css `.badge`.
    if (variant == HeroBadgeVariant.solid) {
      style = style.borderAll(
        color: HeroTokens.colorBackground(),
        width: HeroTokens.doubleBadgeBorderWidth(),
      );
    }
    return style;
  });
}

HeroColor _badgeRole(HeroBadgeColor color) => switch (color) {
      HeroBadgeColor.accent => HeroColor.accent,
      HeroBadgeColor.default_ => HeroColor.default_,
      HeroBadgeColor.success => HeroColor.success,
      HeroBadgeColor.warning => HeroColor.warning,
      HeroBadgeColor.danger => HeroColor.danger,
    };

({ColorToken? fill, ColorToken foreground}) _badgeColors(
  HeroBadgeVariant variant,
  HeroBadgeColor color,
) {
  final t = heroColorTokens(_badgeRole(color));
  return switch (variant) {
    HeroBadgeVariant.solid => (fill: t.fill, foreground: t.fillForeground),
    HeroBadgeVariant.secondary => (fill: HeroTokens.colorDefault, foreground: t.softForeground),
    HeroBadgeVariant.soft => (fill: t.soft, foreground: t.softForeground),
  };
}

/// A HeroUI v3 badge (badge.css) — a small status pill.
class HeroBadge extends StatelessWidget {
  const HeroBadge({
    super.key,
    required this.label,
    this.variant = HeroBadgeVariant.secondary,
    this.color = HeroBadgeColor.default_,
    this.size = HeroBadgeSize.md,
  });

  final String label;
  final HeroBadgeVariant variant;
  final HeroBadgeColor color;
  final HeroBadgeSize size;

  @override
  Widget build(BuildContext context) {
    return RemixBadge(
      style: heroBadgeStyle(variant: variant, color: color, size: size),
      label: label,
    );
  }
}
