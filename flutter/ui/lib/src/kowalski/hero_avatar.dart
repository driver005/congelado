import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../foundation/hero_color_roles.dart';
import '../tokens/hero_tokens.dart';

/// HeroUI v3 avatar sizes (avatar.css `.avatar--sm/md/lg`).
enum HeroAvatarSize { sm, md, lg }

/// HeroUI v3 avatar fallback colors (avatar.css `.avatar__fallback--*`).
enum HeroAvatarColor { accent, default_, success, warning, danger }

final Map<(HeroAvatarSize, HeroAvatarColor), RemixAvatarStyle> _heroAvatarStyleCache = {};

/// Returns the [RemixAvatarStyle] for a HeroUI v3 avatar.
///
/// avatar.css: `.avatar` — `size-10` (40), `rounded-3xl` (24), `bg-default`;
/// `--sm` (32, `rounded-2xl` 16), `--lg` (48, `rounded-3xl`, `text-base`).
/// Fallback text uses the soft foreground of the chosen color.
RemixAvatarStyle heroAvatarStyle({
  HeroAvatarSize size = HeroAvatarSize.md,
  HeroAvatarColor color = HeroAvatarColor.default_,
}) {
  return _heroAvatarStyleCache.putIfAbsent((size, color), () {
    final (diameter, radius, fontSize) = switch (size) {
      HeroAvatarSize.sm => (
          HeroTokens.doubleAvatarSizeSm(),
          HeroTokens.radius2xl(),
          HeroTokens.doubleAvatarFallbackFontSizeSm(),
        ),
      HeroAvatarSize.md => (
          HeroTokens.doubleAvatarSizeMd(),
          HeroTokens.radius3xl(),
          HeroTokens.doubleAvatarFallbackFontSizeMd(),
        ),
      HeroAvatarSize.lg => (
          HeroTokens.doubleAvatarSizeLg(),
          HeroTokens.radius3xl(),
          HeroTokens.doubleAvatarFallbackFontSizeLg(),
        ),
    };
    final role = switch (color) {
      HeroAvatarColor.accent => HeroColor.accent,
      HeroAvatarColor.default_ => HeroColor.default_,
      HeroAvatarColor.success => HeroColor.success,
      HeroAvatarColor.warning => HeroColor.warning,
      HeroAvatarColor.danger => HeroColor.danger,
    };
    final foreground = heroColorTokens(role).softForeground();
    return RemixAvatarStyle()
        .square(diameter)
        .borderRadius(BorderRadiusGeometryMix.all(radius))
        .backgroundColor(HeroTokens.colorDefault())
        .textColor(foreground)
        .iconColor(foreground);
  });
}

/// A HeroUI v3 avatar (avatar.css).
class HeroAvatar extends StatelessWidget {
  const HeroAvatar({
    super.key,
    this.size = HeroAvatarSize.md,
    this.color = HeroAvatarColor.default_,
    this.backgroundImage,
    this.label,
    this.icon,
    this.child,
  });

  final HeroAvatarSize size;
  final HeroAvatarColor color;

  /// Image provider for the avatar image (`.avatar__image`).
  final ImageProvider? backgroundImage;

  /// Fallback text (initials), styled with `.avatar__fallback`.
  final String? label;

  /// Fallback icon.
  final IconData? icon;

  /// Custom child (takes precedence over label/icon).
  final Widget? child;

  @override
  Widget build(BuildContext context) {
    return RemixAvatar(
      style: heroAvatarStyle(size: size, color: color),
      backgroundImage: backgroundImage,
      label: label,
      icon: icon,
      child: child,
    );
  }
}
