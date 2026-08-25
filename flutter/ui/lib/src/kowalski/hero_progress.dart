import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../foundation/hero_color_roles.dart';
import '../tokens/hero_tokens.dart';

/// HeroUI v3 progress bar colors (progress-bar.css `--progress-bar-fill`).
enum HeroProgressColor { accent, default_, success, warning, danger }

final Map<HeroProgressColor, RemixProgressStyle> _heroProgressStyleCache = {};

/// Returns the [RemixProgressStyle] for a HeroUI v3 progress bar.
///
/// progress-bar.css: `.progress-bar__track` — `h-2` (8), `rounded-sm` (4),
/// `bg-default`; `.progress-bar__fill` — fill color, `rounded-sm`, width
/// transition 300ms `--ease-out`.
RemixProgressStyle heroProgressStyle({
  HeroProgressColor color = HeroProgressColor.accent,
}) {
  return _heroProgressStyleCache.putIfAbsent(color, () {
    final role = switch (color) {
      HeroProgressColor.accent => HeroColor.accent,
      HeroProgressColor.default_ => HeroColor.default_,
      HeroProgressColor.success => HeroColor.success,
      HeroProgressColor.warning => HeroColor.warning,
      HeroProgressColor.danger => HeroColor.danger,
    };
    final fill = heroColorTokens(role).fill();
    return RemixProgressStyle(
      animation: AnimationConfig.curve(
        duration: const Duration(milliseconds: heroProgressTransitionMs),
        curve: heroEaseOut,
      ),
    )
        .height(HeroTokens.doubleProgressTrackHeight())
        .trackColor(HeroTokens.colorDefault())
        .track(
          BoxStyler().borderRadiusAll(HeroTokens.radiusProgressRadius()),
        )
        .indicatorColor(fill)
        .indicator(
          BoxStyler().borderRadiusAll(HeroTokens.radiusProgressRadius()),
        );
  });
}

/// A HeroUI v3 determinate progress bar (progress-bar.css).
class HeroProgress extends StatelessWidget {
  const HeroProgress({
    super.key,
    required this.value,
    this.color = HeroProgressColor.accent,
  }) : assert(value >= 0 && value <= 1);

  /// Progress between 0 and 1.
  final double value;

  final HeroProgressColor color;

  @override
  Widget build(BuildContext context) {
    return RemixProgress(
      value: value,
      style: heroProgressStyle(color: color),
    );
  }
}
