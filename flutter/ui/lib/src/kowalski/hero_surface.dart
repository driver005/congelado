import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 surface variants (surface.css `.surface--*`).
enum HeroSurfaceVariant { default_, secondary, tertiary, transparent }

/// A HeroUI v3 surface (surface.css `.surface`) — a styled container with a
/// background tint and matching foreground text color.
class HeroSurface extends StatelessWidget {
  const HeroSurface({
    super.key,
    required this.child,
    this.variant = HeroSurfaceVariant.default_,
    this.padding,
  });

  final Widget child;
  final HeroSurfaceVariant variant;
  final EdgeInsetsGeometry? padding;

  @override
  Widget build(BuildContext context) {
    final (background, foreground) = switch (variant) {
      HeroSurfaceVariant.default_ => (
          HeroTokens.colorSurface,
          HeroTokens.colorSurfaceForeground,
        ),
      HeroSurfaceVariant.secondary => (
          HeroTokens.colorSurfaceSecondary,
          HeroTokens.colorSurfaceSecondaryForeground,
        ),
      HeroSurfaceVariant.tertiary => (
          HeroTokens.colorSurfaceTertiary,
          HeroTokens.colorSurfaceTertiaryForeground,
        ),
      HeroSurfaceVariant.transparent => (
          HeroTokens.colorTransparent,
          HeroTokens.colorForeground,
        ),
    };
    return Container(
      padding: padding,
      decoration: BoxDecoration(
        color: background.resolve(context),
      ),
      child: DefaultTextStyle.merge(
        style: TextStyle(color: foreground.resolve(context)),
        child: child,
      ),
    );
  }
}
