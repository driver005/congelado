import 'package:flutter/material.dart';

import '../foundation/hero_color_roles.dart';
import '../tokens/hero_tokens.dart';

/// HeroUI v3 alert colors (alert.css `.alert--<color>`).
///
/// The alert keeps the surface background; only the indicator + title are
/// tinted. `default` uses the plain foreground, the rest use the role's soft
/// foreground (alert.css has no `--soft` modifier — v3 dropped it).
enum HeroAlertColor { accent, default_, success, warning, danger }

/// A HeroUI v3 alert — a colored banner with an optional icon, title and
/// description (alert.css `.alert`).
///
/// alert.css: `.alert` — `bg-surface px-4 py-3 shadow-surface`, radius
/// `min(32px, var(--radius-3xl))` = 24, `gap-4`; `.alert__title` —
/// `text-sm leading-6 font-medium` tinted per variant; `.alert__description`
/// — `text-sm text-muted`; `.alert__indicator` — `p-1` with a `size-4` icon.
class HeroAlert extends StatelessWidget {
  const HeroAlert({
    super.key,
    this.color = HeroAlertColor.default_,
    this.title,
    this.description,
    this.icon,
  });

  /// Color class — see [HeroAlertColor].
  final HeroAlertColor color;

  /// The alert title (`.alert__title`).
  final String? title;

  /// The alert description (`.alert__description`).
  final String? description;

  /// Optional leading icon (`.alert__indicator`), 16px — the same size as the
  /// CSS `size-4` slot.
  final IconData? icon;

  @override
  Widget build(BuildContext context) {
    final accentColor = _heroAlertAccentColor(context, color);
    final bodyTextSize = HeroTokens.typeSm.resolve(context).fontSize!;

    return Container(
      // alert.css: `bg-surface px-4 py-3 shadow-surface`, radius
      // `min(32px, var(--radius-3xl))`.
      decoration: BoxDecoration(
        color: HeroTokens.colorSurface.resolve(context),
        borderRadius: BorderRadius.circular(
          HeroTokens.radius3xl.resolve(context).x,
        ),
        boxShadow: HeroTokens.shadowSurface.resolve(context),
      ),
      padding: EdgeInsets.symmetric(
        horizontal: HeroTokens.space4.resolve(context),
        vertical: HeroTokens.space3.resolve(context),
      ),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          if (icon != null) ...[
            // `.alert__indicator` — `p-1 select-none`, icon `size-4`.
            Padding(
              padding: EdgeInsets.all(HeroTokens.space1.resolve(context)),
              child: Icon(icon, size: 16, color: accentColor),
            ),
            SizedBox(width: HeroTokens.space4.resolve(context)), // gap-4
          ],
          Expanded(
            // `.alert__content` — `flex flex-col items-start`.
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                if (title != null)
                  Text(
                    title!,
                    // `.alert__title` — `text-sm leading-6 font-medium`.
                    style: TextStyle(
                      fontSize: bodyTextSize,
                      height: 24.0 / bodyTextSize,
                      fontWeight: HeroTokens.weightMedium.resolve(context),
                      color: accentColor,
                    ),
                  ),
                if (description != null)
                  // `.alert__description` — `text-sm text-muted`.
                  Text(
                    description!,
                    style: TextStyle(
                      fontSize: bodyTextSize,
                      color: HeroTokens.colorMuted.resolve(context),
                    ),
                  ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}

/// `.alert--default` tints with the foreground; every other variant uses the
/// role's soft foreground (`.alert--<color> .alert__title/__indicator`).
Color _heroAlertAccentColor(BuildContext context, HeroAlertColor color) {
  return switch (color) {
    HeroAlertColor.default_ => HeroTokens.colorForeground.resolve(context),
    HeroAlertColor.accent => heroColorTokens(
      HeroColor.accent,
    ).softForeground.resolve(context),
    HeroAlertColor.success => heroColorTokens(
      HeroColor.success,
    ).softForeground.resolve(context),
    HeroAlertColor.warning => heroColorTokens(
      HeroColor.warning,
    ).softForeground.resolve(context),
    HeroAlertColor.danger => heroColorTokens(
      HeroColor.danger,
    ).softForeground.resolve(context),
  };
}
