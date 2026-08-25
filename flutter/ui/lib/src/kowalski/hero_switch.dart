import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 switch sizes (switch.css `.switch--sm/md/lg`).
enum HeroSwitchSize { sm, md, lg }

final Map<HeroSwitchSize, RemixSwitchStyle> _heroSwitchStyleCache = {};

/// Returns the [RemixSwitchStyle] for a HeroUI v3 switch.
///
/// switch.css: `.switch__control` — track (default), `rounded-xl` (12),
/// checked `bg-accent`; `.switch__thumb` — `bg-white`, checked
/// `bg-accent-foreground`; sizes sm 32x16, md 40x20, lg 48x24. Hover uses
/// `color-mix(control, transparent 20%)` (approximated with the default
/// hover token); disabled thumbs fade to `default-foreground/20`.
RemixSwitchStyle heroSwitchStyle({HeroSwitchSize size = HeroSwitchSize.md}) {
  return _heroSwitchStyleCache.putIfAbsent(size, () {
    final (controlWidth, controlHeight) = switch (size) {
      HeroSwitchSize.sm => (
          HeroTokens.doubleSwitchControlWidthSm(),
          HeroTokens.doubleSwitchControlHeightSm(),
        ),
      HeroSwitchSize.md => (
          HeroTokens.doubleSwitchControlWidthMd(),
          HeroTokens.doubleSwitchControlHeightMd(),
        ),
      HeroSwitchSize.lg => (
          HeroTokens.doubleSwitchControlWidthLg(),
          HeroTokens.doubleSwitchControlHeightLg(),
        ),
    };
    final controlRadius = switch (size) {
      HeroSwitchSize.sm => HeroTokens.radiusSwitchControlRadiusSm(),
      HeroSwitchSize.md => HeroTokens.radiusSwitchControlRadiusMd(),
      HeroSwitchSize.lg => HeroTokens.radiusSwitchControlRadiusLg(),
    };
    final (thumbWidth, thumbHeight) = switch (size) {
      HeroSwitchSize.sm => (
          HeroTokens.doubleSwitchThumbWidthSm(),
          HeroTokens.doubleSwitchThumbHeightSm(),
        ),
      HeroSwitchSize.md => (
          HeroTokens.doubleSwitchThumbWidthMd(),
          HeroTokens.doubleSwitchThumbHeightMd(),
        ),
      HeroSwitchSize.lg => (
          HeroTokens.doubleSwitchThumbWidthLg(),
          HeroTokens.doubleSwitchThumbHeightLg(),
        ),
    };
    final thumbRadius = switch (size) {
      HeroSwitchSize.sm => HeroTokens.radiusSwitchThumbRadiusSm(),
      HeroSwitchSize.md => HeroTokens.radiusSwitchThumbRadiusMd(),
      HeroSwitchSize.lg => HeroTokens.radiusSwitchThumbRadiusLg(),
    };

    return RemixSwitchStyle(
      container: BoxStyler()
          .width(controlWidth)
          .height(controlHeight)
          .color(HeroTokens.colorDefault())
          .borderRadiusAll(controlRadius),
      thumb: BoxStyler()
          .width(thumbWidth)
          .height(thumbHeight)
          .color(HeroTokens.colorAccentForeground())
          .borderRadiusAll(thumbRadius)
          // switch.css: thumb has a 2px margin on each side (ms-0.5).
          .margin(EdgeInsetsGeometryMix.symmetric(horizontal: 2)),
    )
        .onHovered(
          // switch.css: --switch-control-bg-hover =
          // color-mix(in oklab, var(--switch-control-bg), transparent 20%)
          RemixSwitchStyle().container(
            BoxStyler().color(HeroTokens.colorDefault().withValues(alpha: 0.8)),
          ),
        )
        .onSelected(
          RemixSwitchStyle()
              .container(BoxStyler().color(HeroTokens.colorAccent()))
              // Checked thumb: bg-accent-foreground + soft shadow (switch.css).
              .thumb(
                BoxStyler()
                    .color(HeroTokens.colorAccentForeground())
                    .shadow(
                      BoxShadowMix()
                          .color(HeroTokens.colorForeground().withValues(alpha: 0.06))
                          .offset(x: 0, y: 2)
                          .blurRadius(10),
                    ),
              ),
        )
        .onDisabled(
          RemixSwitchStyle().thumb(
            BoxStyler().color(HeroTokens.colorDefaultForeground().withValues(alpha: 0.2)),
          ),
        );
  });
}

/// A HeroUI v3 switch (switch.css).
class HeroSwitch extends StatelessWidget {
  const HeroSwitch({
    super.key,
    required this.selected,
    required this.onChanged,
    this.size = HeroSwitchSize.md,
    this.enabled = true,
  });

  final bool selected;
  final ValueChanged<bool> onChanged;
  final HeroSwitchSize size;
  final bool enabled;

  @override
  Widget build(BuildContext context) {
    return RemixSwitch(
      selected: selected,
      onChanged: onChanged,
      enabled: enabled,
      style: heroSwitchStyle(size: size),
      // HeroUI is flat — no Material ink ripple.
      enableFeedback: false,
    );
  }
}
