import 'dart:async';
import 'dart:math' as math;

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../tokens/hero_tokens.dart';
import 'hero_anchored_overlay.dart';
import 'hero_button.dart';
import 'hero_color_area.dart';
import 'hero_color_field.dart';
import 'hero_color_slider.dart';
import 'hero_color_swatch.dart';
import 'hero_color_swatch_picker.dart';
import 'hero_focus_ring.dart';

/// Payload colors of the default swatch palette (the colors being offered).
/// Constructed from HSV so no raw named or literal colors leak into
/// chrome roles.
Color _hsvColor(double alpha, double hue, double saturation, double value) =>
    HSVColor.fromAHSV(alpha, hue, saturation, value).toColor();

List<Color> heroDefaultColorSwatches() => [
      _hsvColor(1, 350, 0.78, 0.95), // red
      _hsvColor(1, 25, 0.9, 1), // orange
      _hsvColor(1, 45, 1, 1), // amber
      _hsvColor(1, 135, 0.65, 0.85), // green
      _hsvColor(1, 170, 0.6, 0.85), // teal
      _hsvColor(1, 200, 0.9, 0.9), // cyan
      _hsvColor(1, 215, 0.85, 0.95), // blue
      _hsvColor(1, 265, 0.6, 0.95), // violet
      _hsvColor(1, 320, 0.55, 0.95), // magenta
      _hsvColor(1, 0, 0, 0.55), // gray
    ];

/// Shows the HeroUI v3 color picker (color-picker.css `.color-picker__popover`)
/// as a modal overlay, mirroring [showHeroModal].
///
/// Content: the SV color area, hue/saturation/lightness/alpha sliders, a hex
/// color field and a swatch row. [onSelected] is called live on every change;
/// the returned future resolves to the final color (or null when dismissed).
///
/// The popover is anchored below [anchorKey] (or the caller's context box when
/// no key is given) via [showHeroAnchoredOverlay] — a popover, not a modal.
/// [anchorKey] must resolve to a box on screen (a trigger widget that owns a
/// GlobalKey).
Future<Color?> showHeroColorPicker(
  BuildContext context, {
  required Color initialColor,
  GlobalKey? anchorKey,
  ValueChanged<Color>? onSelected,
  List<Color>? swatches,
}) {
  final overlay = Overlay.maybeOf(context);
  final targetBox =
      (anchorKey?.currentContext ?? context).findRenderObject() as RenderBox?;
  final overlayBox = overlay?.context.findRenderObject() as RenderBox?;
  if (overlay == null || targetBox == null || overlayBox == null) {
    return Future.value(null);
  }
  final origin = overlayBox.globalToLocal(
    targetBox.localToGlobal(Offset.zero),
  );
  final anchorRect = origin & targetBox.size;

  final completer = Completer<Color?>();
  late HeroAnchoredOverlayClose close;

  final scopeColors = HeroPopoverColors.resolve(context);

  close = showHeroAnchoredOverlay(
    context,
    anchorRect: anchorRect,
    gap: 4,
    onClosed: () {
      if (!completer.isCompleted) completer.complete(null);
    },
    builder: (entryContext) => HeroPopoverPanel(
      colors: scopeColors,
      child: _HeroColorPickerPanel(
        initialColor: initialColor,
        onSelected: onSelected,
        swatches: swatches ?? heroDefaultColorSwatches(),
        onDone: (color) {
          // Complete BEFORE close(): close() fires onClosed synchronously,
          // which would complete with null first (guarded by isCompleted).
          if (!completer.isCompleted) completer.complete(color);
          close();
        },
      ),
    ),
  );

  return completer.future;
}

/// A HeroUI v3 color picker trigger (color-picker.css `.color-picker` /
/// `.color-picker__trigger`) — a swatch (with optional label) that opens
/// [showHeroColorPicker] on tap.
class HeroColorPicker extends StatelessWidget {
  const HeroColorPicker({
    super.key,
    required this.color,
    this.onChanged,
    this.label,
    this.enabled = true,
    this.swatches,
  });

  final Color color;

  /// Called live while the picker changes colors.
  final ValueChanged<Color>? onChanged;

  /// Optional label next to the swatch (`[data-slot="label"]`).
  final String? label;

  final bool enabled;

  /// Custom swatch palette; defaults to [heroDefaultColorSwatches].
  final List<Color>? swatches;

  Future<void> _open(BuildContext context) async {
    final picked = await showHeroColorPicker(
      context,
      initialColor: color,
      onSelected: onChanged,
      swatches: swatches,
    );
    if (picked != null) onChanged?.call(picked);
  }

  @override
  Widget build(BuildContext context) {
    final trigger = HeroFocusRing(
      radius: 4, // rounded-sm
      builder: (context, node, focused) => Focus(
        focusNode: node,
        onKeyEvent: (node, event) => _activateOnKey(event, () => _open(context)),
        child: GestureDetector(
          behavior: HitTestBehavior.opaque,
          onTap: enabled ? () => _open(context) : null,
          child: Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              HeroColorSwatch(color: color, size: HeroColorSwatchSize.md),
              if (label != null) ...[
                const SizedBox(width: 12), // gap-3
                Text(
                  label!,
                  style: TextStyle(
                    fontSize: 14, // text-sm
                    color: HeroTokens.colorForeground.resolve(context),
                  ),
                ),
              ],
            ],
          ),
        ),
      ),
    );

    if (!enabled) {
      return Opacity(
        opacity: HeroTokens.doubleDisabledOpacity.resolve(context),
        child: trigger,
      );
    }
    return trigger;
  }
}

class _HeroColorPickerPanel extends StatefulWidget {
  const _HeroColorPickerPanel({
    required this.initialColor,
    required this.swatches,
    this.onSelected,
    this.onDone,
  });

  final Color initialColor;
  final ValueChanged<Color>? onSelected;

  /// Resolves the picker with the final color (popover close).
  final ValueChanged<Color>? onDone;
  final List<Color> swatches;

  @override
  State<_HeroColorPickerPanel> createState() => _HeroColorPickerPanelState();
}

class _HeroColorPickerPanelState extends State<_HeroColorPickerPanel> {
  late HSVColor _hsv;
  late double _alpha;

  @override
  void initState() {
    super.initState();
    final c = HSVColor.fromColor(widget.initialColor);
    _hsv = HSVColor.fromAHSV(1, c.hue, c.saturation, c.value);
    _alpha = c.alpha;
  }

  Color get _color => HSVColor.fromAHSV(
        _alpha,
        _hsv.hue,
        _hsv.saturation,
        _hsv.value,
      ).toColor();

  void _setHsv(HSVColor next) {
    setState(() => _hsv = next);
    widget.onSelected?.call(_color);
  }

  void _setAlpha(double alpha) {
    setState(() => _alpha = alpha);
    widget.onSelected?.call(_color);
  }

  // HSV -> HSL lightness (and back) for the lightness slider.
  double get _lightness => _hsv.value * (1 - _hsv.saturation / 2);

  double get _hslSaturation {
    final l = _lightness;
    if (l <= 0 || l >= 1) return 0;
    return (_hsv.value - l) / math.min(l, 1 - l);
  }

  HSVColor _fromLightness(double lightness) {
    final s = _hslSaturation;
    final v = lightness + s * math.min(lightness, 1 - lightness);
    final sv = v <= 0 ? 0.0 : 2 * (1 - lightness / v);
    return HSVColor.fromAHSV(
      1,
      _hsv.hue,
      sv.clamp(0.0, 1.0),
      v.clamp(0.0, 1.0),
    );
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          HeroColorArea(
            value: _hsv,
            onChanged: (c) => _setHsv(c),
          ),
          const SizedBox(height: 12), // gap-3
          HeroColorSlider(
            channel: HeroColorSliderChannel.hue,
            value: _hsv.hue / 360,
            color: _hsv,
            label: 'Hue',
            output: '${_hsv.hue.round()}°',
            onChanged: (v) => _setHsv(
              HSVColor.fromAHSV(1, (v * 360) % 360, _hsv.saturation, _hsv.value),
            ),
          ),
          const SizedBox(height: 12),
          HeroColorSlider(
            channel: HeroColorSliderChannel.saturation,
            value: _hsv.saturation,
            color: _hsv,
            label: 'Saturation',
            output: '${(_hsv.saturation * 100).round()}%',
            onChanged: (v) => _setHsv(
              HSVColor.fromAHSV(1, _hsv.hue, v, _hsv.value),
            ),
          ),
          const SizedBox(height: 12),
          HeroColorSlider(
            channel: HeroColorSliderChannel.lightness,
            value: _lightness,
            color: _hsv,
            label: 'Lightness',
            output: '${(_lightness * 100).round()}%',
            onChanged: (v) => _setHsv(_fromLightness(v)),
          ),
          const SizedBox(height: 12),
          HeroColorSlider(
            channel: HeroColorSliderChannel.alpha,
            value: _alpha,
            color: _hsv,
            label: 'Alpha',
            output: '${(_alpha * 100).round()}%',
            onChanged: _setAlpha,
          ),
          const SizedBox(height: 12),
          HeroColorField(
            value: _color,
            fullWidth: true,
            onChanged: (c) {
              if (c != null) {
                final hsv = HSVColor.fromColor(c);
                setState(() {
                  _hsv = HSVColor.fromAHSV(1, hsv.hue, hsv.saturation, hsv.value);
                  _alpha = hsv.alpha;
                });
                widget.onSelected?.call(c);
              }
            },
          ),
          const SizedBox(height: 12),
          HeroColorSwatchPicker(
            colors: widget.swatches,
            value: _color,
            onChanged: (c) {
              final hsv = HSVColor.fromColor(c);
              setState(() {
                _hsv = HSVColor.fromAHSV(1, hsv.hue, hsv.saturation, hsv.value);
                _alpha = hsv.alpha;
              });
              widget.onSelected?.call(c);
            },
          ),
          const SizedBox(height: 4),
          Align(
            alignment: AlignmentDirectional.centerEnd,
            child: HeroButton(
              label: 'Done',
              size: HeroButtonSize.sm,
              onPressed: () => widget.onDone?.call(_color),
            ),
          ),
        ],
      ),
    );
  }
}

KeyEventResult _activateOnKey(KeyEvent event, VoidCallback onTap) {
  if (event is! KeyDownEvent) return KeyEventResult.ignored;
  if (event.logicalKey == LogicalKeyboardKey.enter ||
      event.logicalKey == LogicalKeyboardKey.space) {
    onTap();
    return KeyEventResult.handled;
  }
  return KeyEventResult.ignored;
}
