import 'package:flutter/material.dart';

import '../foundation/hero_color_roles.dart';
import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';

/// HeroUI v3 meter colors (meter.css `.meter--<color>` → `--meter-fill`).
enum HeroMeterColor { accent, default_, success, warning, danger }

/// HeroUI v3 meter sizes (meter.css `.meter--sm/md/lg` — track height and
/// radius).
enum HeroMeterSize { sm, md, lg }

/// A HeroUI v3 meter — a quantity indicator within a known range
/// (meter.css `.meter`).
///
/// The spec's grid (`grid-template-areas: "label output" / "track track"`) is
/// a label row on top and the track below it. The fill animates its width
/// `300ms var(--ease-out)` (motion-reduce aware) and takes the role fill
/// (`--default` uses `--default-foreground`); the track is `bg-default`.
class HeroMeter extends StatelessWidget {
  const HeroMeter({
    super.key,
    required this.value,
    this.color = HeroMeterColor.accent,
    this.size = HeroMeterSize.md,
    this.label,
    this.output,
    this.disabled = false,
  }) : assert(value >= 0 && value <= 1);

  /// Progress between 0 and 1.
  final double value;

  /// Color class — see [HeroMeterColor].
  final HeroMeterColor color;

  /// Size — see [HeroMeterSize].
  final HeroMeterSize size;

  /// Optional label (`[data-slot="label"]` — `text-sm font-medium`).
  final String? label;

  /// Optional output text (`[data-slot="output"]` — `text-sm font-medium
  /// tabular-nums`); defaults to the percent.
  final String? output;

  /// Disabled state (`status-disabled`; the label keeps full opacity).
  final bool disabled;

  @override
  Widget build(BuildContext context) {
    final (trackHeight, trackRadius) = switch (size) {
      // meter.css: --sm `h-1 rounded-xs`, default (--md) `h-2 rounded-sm`,
      // --lg `h-3 rounded-md`.
      HeroMeterSize.sm => (
        HeroTokens.space1.resolve(context),
        HeroTokens.radiusXs.resolve(context).x,
      ),
      HeroMeterSize.md => (
        HeroTokens.space2.resolve(context),
        HeroTokens.radiusSm.resolve(context).x,
      ),
      HeroMeterSize.lg => (
        HeroTokens.space3.resolve(context),
        HeroTokens.radiusMd.resolve(context).x,
      ),
    };
    final fill = switch (color) {
      HeroMeterColor.default_ => HeroTokens.colorDefaultForeground.resolve(
        context,
      ),
      HeroMeterColor.accent => heroColorTokens(
        HeroColor.accent,
      ).fill.resolve(context),
      HeroMeterColor.success => heroColorTokens(
        HeroColor.success,
      ).fill.resolve(context),
      HeroMeterColor.warning => heroColorTokens(
        HeroColor.warning,
      ).fill.resolve(context),
      HeroMeterColor.danger => heroColorTokens(
        HeroColor.danger,
      ).fill.resolve(context),
    };
    final bodySize = HeroTokens.typeSm.resolve(context).fontSize;
    final disabledOpacity = HeroTokens.doubleDisabledOpacity.resolve(context);

    Widget track = SizedBox(
      height: trackHeight,
      child: ClipRRect(
        borderRadius: BorderRadius.circular(trackRadius),
        child: Stack(
          fit: StackFit.expand,
          children: [
            // `.meter__track` — `bg-default`.
            ColoredBox(color: HeroTokens.colorDefault.resolve(context)),
            // `.meter__fill` — `width` = value, 300ms ease-out.
            TweenAnimationBuilder<double>(
              tween: Tween(begin: 0, end: value),
              duration: HeroMotion.durationOf(
                context,
                const Duration(milliseconds: 300),
              ),
              curve: heroEaseOut,
              builder: (context, v, _) => FractionallySizedBox(
                alignment: Alignment.centerLeft,
                widthFactor: v,
                child: ColoredBox(color: fill),
              ),
            ),
          ],
        ),
      ),
    );

    final body = Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        if (label != null || output != null) ...[
          Row(
            children: [
              if (label != null)
                // `[data-slot="label"]` — `text-sm font-medium`; stays at
                // full opacity when disabled.
                Expanded(
                  child: Text(
                    label!,
                    style: TextStyle(
                      fontSize: bodySize,
                      fontWeight: HeroTokens.weightMedium.resolve(context),
                      color: HeroTokens.colorForeground.resolve(context),
                    ),
                  ),
                )
              else
                // Grid `1fr auto`: the output sits in the trailing column.
                const Spacer(),
              Opacity(
                opacity: disabled ? disabledOpacity : 1.0,
                // `[data-slot="output"]` — `text-sm font-medium tabular-nums`.
                child: Text(
                  output ?? '${(value * 100).round()}%',
                  style: TextStyle(
                    fontSize: bodySize,
                    fontWeight: HeroTokens.weightMedium.resolve(context),
                    fontFeatures: const [FontFeature.tabularFigures()],
                    color: HeroTokens.colorForeground.resolve(context),
                  ),
                ),
              ),
            ],
          ),
          SizedBox(height: HeroTokens.space1.resolve(context)), // gap-1
        ],
        Opacity(opacity: disabled ? disabledOpacity : 1.0, child: track),
      ],
    );

    return body;
  }
}
