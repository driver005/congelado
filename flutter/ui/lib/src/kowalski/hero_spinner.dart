import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../foundation/hero_color_roles.dart';
import '../tokens/hero_tokens.dart';

/// HeroUI v3 spinner colors (spinner.css `.spinner--<color>`).
enum HeroSpinnerColor { current, accent, danger, success, warning }

/// HeroUI v3 spinner sizes (spinner.css `.spinner--sm/md/lg/xl`).
enum HeroSpinnerSize { sm, md, lg, xl }

final Map<(HeroSpinnerColor, HeroSpinnerSize, Color?), RemixSpinnerStyle>
    _heroSpinnerStyleCache = {};

/// Returns the [RemixSpinnerStyle] for a HeroUI v3 spinner.
///
/// spinner.css: `.spinner` — `size-6` (24) default, `--sm` 16, `--lg` 32,
/// `--xl` 40; `animate-spin-fast` (0.75s). `--current` inherits the ambient
/// text color, the rest use the status colors.
RemixSpinnerStyle heroSpinnerStyle({
  HeroSpinnerColor color = HeroSpinnerColor.current,
  HeroSpinnerSize size = HeroSpinnerSize.md,
  Color? indicatorColor,
}) {
  return _heroSpinnerStyleCache.putIfAbsent(
    (color, size, indicatorColor),
    () {
      final indicator = switch (color) {
        HeroSpinnerColor.current => null, // inherit ambient color
        HeroSpinnerColor.accent => heroColorTokens(HeroColor.accent).fill(),
        HeroSpinnerColor.danger => heroColorTokens(HeroColor.danger).fill(),
        HeroSpinnerColor.success => heroColorTokens(HeroColor.success).fill(),
        HeroSpinnerColor.warning => heroColorTokens(HeroColor.warning).fill(),
      };
      final diameter = switch (size) {
        HeroSpinnerSize.sm => HeroTokens.doubleSpinnerSizeSm(),
        HeroSpinnerSize.md => HeroTokens.doubleSpinnerSizeMd(),
        HeroSpinnerSize.lg => HeroTokens.doubleSpinnerSizeLg(),
        HeroSpinnerSize.xl => HeroTokens.doubleSpinnerSizeXl(),
      };
      return RemixSpinnerStyle(
        size: diameter,
        strokeWidth: HeroTokens.doubleSpinnerStrokeWidth(),
        indicatorColor: indicatorColor ?? indicator,
        duration: const Duration(milliseconds: heroSpinMs),
      );
    },
  );
}

/// A HeroUI v3 spinner (spinner.css).
class HeroSpinner extends StatelessWidget {
  const HeroSpinner({
    super.key,
    this.color = HeroSpinnerColor.current,
    this.size = HeroSpinnerSize.md,
  });

  final HeroSpinnerColor color;
  final HeroSpinnerSize size;

  @override
  Widget build(BuildContext context) {
    // `--current` inherits the ambient text color (spinner.css); Remix
    // falls back to Theme.colorScheme.primary when no color is set, which is
    // not the HeroUI meaning of current.
    final current = color == HeroSpinnerColor.current
        ? DefaultTextStyle.of(context).style.color
        : null;
    return RemixSpinner(
      style: heroSpinnerStyle(
        color: color,
        size: size,
        indicatorColor: current,
      ),
    );
  }
}
