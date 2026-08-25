import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 card variants (card.css `.card--*`).
enum HeroCardVariant {
  /// Surface background (`.card--default`).
  surface,

  /// Secondary surface background (`.card--secondary`).
  secondary,

  /// Tertiary surface background (`.card--tertiary`).
  tertiary,

  /// No background, no shadow (`.card--transparent`).
  transparent,
}

final Map<HeroCardVariant, RemixCardStyle> _heroCardStyleCache = {};

/// Returns the [RemixCardStyle] for a HeroUI v3 card.
///
/// card.css: `.card` — `p-4` (16), `gap-3` (12), radius `min(32px,
/// var(--radius-3xl))` = 24, `shadow-surface`. The shadow is per-theme
/// (dark mode has no surface shadow), so the facade resolves it from the
/// scope and merges it onto the base recipe.
RemixCardStyle heroCardStyle({HeroCardVariant variant = HeroCardVariant.surface}) {
  return _heroCardStyleCache.putIfAbsent(variant, () {
    final base = RemixCardStyle()
        .padding(EdgeInsetsGeometryMix.all(HeroTokens.space4()))
        .borderRadius(BorderRadiusGeometryMix.all(HeroTokens.radius3xl()));

    if (variant == HeroCardVariant.transparent) {
      return base.color(HeroTokens.colorTransparent());
    }
    return base.color(switch (variant) {
      HeroCardVariant.surface => HeroTokens.colorSurface(),
      HeroCardVariant.secondary => HeroTokens.colorSurfaceSecondary(),
      HeroCardVariant.tertiary => HeroTokens.colorSurfaceTertiary(),
      HeroCardVariant.transparent => HeroTokens.colorTransparent(),
    });
  });
}

/// A HeroUI v3 card.
///
/// ```dart
/// HeroCard(
///   child: Column(
///     crossAxisAlignment: CrossAxisAlignment.start,
///     children: [
///       const HeroCardTitle('Product'),
///       const HeroCardDescription('Details about this product.'),
///       const SizedBox(height: 8),
///       Text('Card content goes here.'),
///     ],
///   ),
/// )
/// ```
class HeroCard extends StatelessWidget {
  const HeroCard({
    super.key,
    this.variant = HeroCardVariant.surface,
    this.child,
  });

  final HeroCardVariant variant;
  final Widget? child;

  @override
  Widget build(BuildContext context) {
    // The surface shadow is per-theme (none in dark): resolve it here and
    // merge onto the theme-agnostic base recipe.
    final shadows = HeroTokens.shadowSurface.resolve(context);
    final style = shadows.isEmpty
        ? heroCardStyle(variant: variant)
        : heroCardStyle(variant: variant)
            .boxShadows([for (final s in shadows) BoxShadowMix.value(s)]);
    return RemixCard(style: style, child: child);
  }
}

/// HeroUI v3 card title typography (`.card__title`: `text-sm leading-6
/// font-medium`).
class HeroCardTitle extends StatelessWidget {
  const HeroCardTitle(this.text, {super.key, this.textAlign});

  final String text;
  final TextAlign? textAlign;

  @override
  Widget build(BuildContext context) {
    return Text(
      text,
      textAlign: textAlign,
      style: TextStyle(
        fontSize: HeroTokens.doubleCardTitleFontSize.resolve(context),
        height: HeroTokens.doubleCardTitleLineHeight.resolve(context) /
            HeroTokens.doubleCardTitleFontSize.resolve(context),
        fontWeight: HeroTokens.weightMedium.resolve(context),
        color: HeroTokens.colorForeground.resolve(context),
      ),
    );
  }
}

/// HeroUI v3 card description typography (`.card__description`: `text-sm
/// leading-5 text-muted`).
class HeroCardDescription extends StatelessWidget {
  const HeroCardDescription(this.text, {super.key, this.textAlign});

  final String text;
  final TextAlign? textAlign;

  @override
  Widget build(BuildContext context) {
    return Text(
      text,
      textAlign: textAlign,
      style: TextStyle(
        fontSize: HeroTokens.doubleCardDescriptionFontSize.resolve(context),
        height: HeroTokens.doubleCardDescriptionLineHeight.resolve(context) /
            HeroTokens.doubleCardDescriptionFontSize.resolve(context),
        color: HeroTokens.colorMuted.resolve(context),
      ),
    );
  }
}
