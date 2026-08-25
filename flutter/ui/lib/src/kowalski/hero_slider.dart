import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// A HeroUI v3 slider (slider.css `.slider`) — a filled track with a draggable
/// thumb, an optional label and an output value.
///
/// slider.css: `.slider__track` — `h-5 rounded-xl bg-default` with transparent
/// 0.75rem side borders so the thumb can overhang; `.slider__fill` —
/// `bg-accent`; `.slider__thumb` — `bg-accent rounded-xl` outer with a
/// `rounded-lg bg-accent-foreground shadow-field` inner pill (24×16), scaled to
/// 0.9 while dragging. Disabled fades the whole slider to `--disabled-opacity`.
class HeroSlider extends StatefulWidget {
  const HeroSlider({
    super.key,
    this.value = 0,
    this.onChanged,
    this.onChangeEnd,
    this.min = 0,
    this.max = 100,
    this.step,
    this.label,
    this.showOutput = true,
    this.formatOutput,
    this.enabled = true,
    this.fullWidth = true,
  });

  final double value;
  final ValueChanged<double>? onChanged;
  final ValueChanged<double>? onChangeEnd;
  final double min;
  final double max;

  /// Optional discrete step (slider snaps to multiples of [step]).
  final double? step;

  /// Optional label shown above the track (`text-sm font-medium`).
  final String? label;

  /// Whether to show the current value on the right of the label row.
  final bool showOutput;

  final String Function(double value)? formatOutput;

  final bool enabled;
  final bool fullWidth;

  @override
  State<HeroSlider> createState() => _HeroSliderState();
}

class _HeroSliderState extends State<HeroSlider> {
  static const double _thumbOverhang = 12.0; // border-x-[0.75rem] transparent
  static const double _thumbWidth = 28.0; // 1.5rem + 0.25rem

  bool _dragging = false;

  double get _range => widget.max - widget.min;

  double _snap(double value) {
    final step = widget.step;
    if (step == null || step <= 0) return value.clamp(widget.min, widget.max);
    final stepped = (value / step).round() * step;
    return stepped.clamp(widget.min, widget.max);
  }

  double _fracFromDx(double dx, double trackWidth) {
    final frac = ((dx - _thumbOverhang) / trackWidth).clamp(0.0, 1.0);
    return frac;
  }

  double _valueFromFrac(double frac) =>
      _snap(widget.min + _range * frac.clamp(0.0, 1.0));

  void _setFromDx(double dx, double trackWidth, {bool end = false}) {
    final value = _valueFromFrac(_fracFromDx(dx, trackWidth));
    widget.onChanged?.call(value);
    if (end) widget.onChangeEnd?.call(value);
  }

  @override
  Widget build(BuildContext context) {
    final opacity =
        widget.enabled ? 1.0 : HeroTokens.doubleDisabledOpacity.resolve(context);
    final frac = _range == 0 ? 0.0 : ((widget.value - widget.min) / _range).clamp(0.0, 1.0);

    return Opacity(
      opacity: opacity,
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            children: [
              if (widget.label != null)
                Text(
                  widget.label!,
                  style: TextStyle(
                    fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                    fontWeight: HeroTokens.weightMedium.resolve(context),
                    color: HeroTokens.colorForeground.resolve(context),
                  ),
                ),
              const Spacer(),
              if (widget.showOutput)
                Text(
                  widget.formatOutput?.call(widget.value) ??
                      _format(widget.value),
                  style: TextStyle(
                    fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                    fontWeight: HeroTokens.weightMedium.resolve(context),
                    color: HeroTokens.colorForeground.resolve(context),
                    fontFeatures: const [FontFeature.tabularFigures()],
                  ),
                ),
            ],
          ),
          SizedBox(height: HeroTokens.space1.resolve(context)),
          HeroFocusRing(
            radius: 8,
            builder: (context, node, focused) => Focus(
              focusNode: node,
              canRequestFocus: widget.enabled,
              onKeyEvent: widget.enabled
                  ? (node, event) => _handleKey(event)
                  : null,
              child: SizedBox(
                height: 20,
                width: widget.fullWidth ? double.infinity : null,
                child: LayoutBuilder(
                  builder: (context, constraints) {
                    final trackWidth = (constraints.maxWidth - _thumbOverhang * 2)
                        .clamp(0.0, double.infinity);
                    final fillWidth = trackWidth * frac;
                    final thumbLeft =
                        _thumbOverhang + trackWidth * frac - _thumbWidth / 2;
                    return GestureDetector(
                      behavior: HitTestBehavior.opaque,
                      onHorizontalDragStart: widget.enabled
                          ? (d) => setState(() => _dragging = true)
                          : null,
                      onHorizontalDragUpdate: widget.enabled
                          ? (d) => _setFromDx(d.localPosition.dx, trackWidth)
                          : null,
                      onHorizontalDragEnd: widget.enabled
                          ? (d) {
                              setState(() => _dragging = false);
                              widget.onChangeEnd?.call(widget.value);
                            }
                          : null,
                      onTapDown: widget.enabled
                          ? (d) => _setFromDx(
                                d.localPosition.dx,
                                trackWidth,
                                end: true,
                              )
                          : null,
                      child: Stack(
                        children: [
                          // Track: h-5 rounded-xl bg-default (with the
                          // transparent overhang zone on both sides).
                          Positioned(
                            left: 0,
                            top: 0,
                            right: 0,
                            bottom: 0,
                            child: Container(
                              decoration: BoxDecoration(
                                color: HeroTokens.colorDefault.resolve(context),
                                borderRadius: BorderRadius.circular(12),
                              ),
                            ),
                          ),
                          // Fill: absolute bg-accent.
                          Positioned(
                            left: _thumbOverhang,
                            top: 0,
                            bottom: 0,
                            width: fillWidth,
                            child: Container(
                              decoration: BoxDecoration(
                                color: HeroTokens.colorAccent.resolve(context),
                                borderRadius: BorderRadius.only(
                                  topLeft: const Radius.circular(12),
                                  bottomLeft: const Radius.circular(12),
                                  topRight: Radius.circular(
                                    fillWidth >= trackWidth ? 12 : 0,
                                  ),
                                  bottomRight: Radius.circular(
                                    fillWidth >= trackWidth ? 12 : 0,
                                  ),
                                ),
                              ),
                            ),
                          ),
                          // Thumb: accent outer (28×20) + accent-foreground
                          // inner pill (24×16, shadow-field).
                          Positioned(
                            left: thumbLeft,
                            top: 0,
                            bottom: 0,
                            width: _thumbWidth,
                            child: Center(
                              child: AnimatedScale(
                                scale: _dragging ? 0.9 : 1.0,
                                duration: HeroMotion.durationOf(
                                  context,
                                  const Duration(milliseconds: 250),
                                ),
                                curve: HeroMotion.out,
                                child: Container(
                                  width: 24,
                                  height: 16,
                                  decoration: BoxDecoration(
                                    color: HeroTokens.colorAccentForeground.resolve(context),
                                    borderRadius: BorderRadius.circular(8),
                                    boxShadow:
                                        HeroTokens.shadowField.resolve(context),
                                  ),
                                ),
                              ),
                            ),
                          ),
                        ],
                      ),
                    );
                  },
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }

  KeyEventResult _handleKey(KeyEvent event) {
    if (event is! KeyDownEvent) return KeyEventResult.ignored;
    final delta = switch (event.logicalKey) {
      LogicalKeyboardKey.arrowLeft ||
      LogicalKeyboardKey.arrowDown =>
        -1,
      LogicalKeyboardKey.arrowRight ||
      LogicalKeyboardKey.arrowUp =>
        1,
      _ => null,
    };
    if (delta == null) return KeyEventResult.ignored;
    final step = widget.step ?? (_range / 100);
    widget.onChanged?.call(_snap(widget.value + delta * step));
    widget.onChangeEnd?.call(_snap(widget.value + delta * step));
    return KeyEventResult.handled;
  }

  String _format(double v) {
    if (v == v.roundToDouble()) return v.toInt().toString();
    return v.toStringAsFixed(2).replaceFirst(RegExp(r'0+$'), '');
  }
}
