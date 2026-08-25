import 'package:flutter/material.dart';

import '../foundation/hero_color_roles.dart';
import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';

/// HeroUI v3 progress-circle colors (progress-circle.css
/// `.progress-circle--<color>` → `--progress-circle-stroke`).
enum HeroProgressCircleColor { accent, default_, success, warning, danger }

/// HeroUI v3 progress-circle sizes (progress-circle.css
/// `.progress-circle--sm/md/lg` → track `size-5/7/9`).
enum HeroProgressCircleSize { sm, md, lg }

/// A HeroUI v3 determinate circular progress indicator
/// (progress-circle.css `.progress-circle`).
///
/// A full track circle (`--progress-circle-track-stroke: var(--default)`)
/// with the value arc on top (`--progress-circle-stroke`, the role fill —
/// `--default` uses `--default-foreground`). When [value] is null the circle
/// is indeterminate and spins (`progress-circle-spin 1s linear infinite`),
/// mirroring `.progress-circle:not([aria-valuenow])`.
class HeroProgressCircle extends StatelessWidget {
  const HeroProgressCircle({
    super.key,
    this.value,
    this.color = HeroProgressCircleColor.accent,
    this.size = HeroProgressCircleSize.md,
    this.label,
    this.valueLabel,
  }) : assert(value == null || (value >= 0 && value <= 1));

  /// Progress between 0 and 1; null renders the indeterminate spinner.
  final double? value;

  /// Color class — see [HeroProgressCircleColor].
  final HeroProgressCircleColor color;

  /// Size — see [HeroProgressCircleSize].
  final HeroProgressCircleSize size;

  /// Optional label shown under the circle (storybook "with label").
  final String? label;

  /// Optional value text shown under the circle (storybook "with label");
  /// defaults to the percent when [label] is set.
  final String? valueLabel;

  @override
  Widget build(BuildContext context) {
    final diameter = switch (size) {
      HeroProgressCircleSize.sm => HeroTokens.space5.resolve(
        context,
      ), // size-5 = 20
      HeroProgressCircleSize.md => 28.0, // size-7 (no 28 token)
      HeroProgressCircleSize.lg => 36.0, // size-9 (no 36 token)
    };
    final stroke = switch (color) {
      HeroProgressCircleColor.default_ =>
        HeroTokens.colorDefaultForeground.resolve(context),
      HeroProgressCircleColor.accent => heroColorTokens(
        HeroColor.accent,
      ).fill.resolve(context),
      HeroProgressCircleColor.success => heroColorTokens(
        HeroColor.success,
      ).fill.resolve(context),
      HeroProgressCircleColor.warning => heroColorTokens(
        HeroColor.warning,
      ).fill.resolve(context),
      HeroProgressCircleColor.danger => heroColorTokens(
        HeroColor.danger,
      ).fill.resolve(context),
    };
    final bodySize = HeroTokens.typeSm.resolve(context).fontSize;

    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        SizedBox(
          width: diameter,
          height: diameter,
          child: TweenAnimationBuilder<double>(
            // progress-circle.css: `stroke-dashoffset 300ms var(--ease-out)`.
            tween: Tween(begin: 0, end: value ?? 0),
            duration: HeroMotion.durationOf(
              context,
              const Duration(milliseconds: 300),
            ),
            curve: heroEaseOut,
            builder: (context, v, _) => CircularProgressIndicator(
              value: value == null ? null : v,
              strokeWidth: 6, // HeroUI ProgressCircle default stroke width
              strokeCap: StrokeCap.round,
              color: stroke,
              backgroundColor: HeroTokens.colorDefault.resolve(context),
              strokeAlign: CircularProgressIndicator.strokeAlignInside,
            ),
          ),
        ),
        if (label != null || valueLabel != null) ...[
          SizedBox(height: HeroTokens.space1.resolve(context)),
          if (label != null)
            Text(
              label!,
              style: TextStyle(
                fontSize: bodySize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorForeground.resolve(context),
              ),
            ),
          if (label != null)
            SizedBox(height: HeroTokens.space05.resolve(context)),
          // React's `valueLabel` defaults to the formatted percent.
          Text(
            valueLabel ?? '${((value ?? 0) * 100).round()}%',
            style: TextStyle(
              fontSize: bodySize,
              fontWeight: HeroTokens.weightMedium.resolve(context),
              color: HeroTokens.colorMuted.resolve(context),
            ),
          ),
        ],
      ],
    );
  }
}
