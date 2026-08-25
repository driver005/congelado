import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 separator colors (separator.css `.separator--*`).
enum HeroDividerColor { default_, secondary, tertiary }

final Map<HeroDividerColor, RemixDividerStyle> _heroDividerStyleCache = {};

/// Returns the [RemixDividerStyle] for a HeroUI v3 separator.
///
/// separator.css: `.separator` — `h-px w-full bg-separator rounded-sm`;
/// `--secondary` / `--tertiary` swap the color levels.
RemixDividerStyle heroDividerStyle({
  HeroDividerColor color = HeroDividerColor.default_,
}) {
  return _heroDividerStyleCache.putIfAbsent(color, () {
    return RemixDividerStyle()
        .color(switch (color) {
          HeroDividerColor.default_ => HeroTokens.colorSeparator(),
          HeroDividerColor.secondary => HeroTokens.colorSeparatorSecondary(),
          HeroDividerColor.tertiary => HeroTokens.colorSeparatorTertiary(),
        })
        .thickness(HeroTokens.doubleSeparatorThickness());
  });
}

/// A HeroUI v3 divider / separator (separator.css).
class HeroDivider extends StatelessWidget {
  const HeroDivider({
    super.key,
    this.color = HeroDividerColor.default_,
  });

  final HeroDividerColor color;

  @override
  Widget build(BuildContext context) {
    return RemixDivider(style: heroDividerStyle(color: color));
  }
}
