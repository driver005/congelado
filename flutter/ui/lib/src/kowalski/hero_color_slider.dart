import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// HeroUI v3 color slider channels (color-slider.css). The track gradient is
/// data (painted per channel, mirroring the JS-injected `--track-start-color`
/// / `--track-end-color`), the caps + chrome use role tokens.
enum HeroColorSliderChannel { hue, saturation, lightness, alpha }

/// Payload colors of the slider track (the channel face being displayed).
/// Constructed from HSV so no raw named or literal colors leak into
/// chrome roles.
Color _hsvColor(double alpha, double hue, double saturation, double value) =>
    HSVColor.fromAHSV(alpha, hue, saturation, value).toColor();

HSVColor _hsv(double alpha, double hue, double saturation, double value) =>
    HSVColor.fromAHSV(alpha, hue, saturation, value);

final Color _heroSliderWhite = _hsvColor(1, 0, 0, 1);
final Color _heroSliderBlack = _hsvColor(1, 0, 0, 0);
final Color _heroSliderCheckerLight = _hsvColor(1, 0, 0, 0.969);
final Color _heroSliderCheckerDark = _hsvColor(1, 0, 0, 0.937);

Color _heroSliderHue(double degrees) => _hsvColor(1, degrees % 360, 1, 1);

/// A HeroUI v3 color slider (color-slider.css) — a gradient track for one
/// color channel with a white-ringed thumb.
///
/// Geometry mirrors `.color-slider` / `[data-orientation="horizontal"]`:
/// 20px track (`h-5`) with 10px rounded edge caps (`0.625rem`), 16px thumb
/// (`size-4`, `border-3 border-white`, `shadow-overlay`, `rounded-2xl`).
///
/// Vertical orientation fills 160px by default; wrap it in a taller
/// [SizedBox] to give the track more length (the CSS uses `h-full`).
class HeroColorSlider extends StatefulWidget {
  const HeroColorSlider({
    super.key,
    required this.channel,
    required this.value,
    required this.onChanged,
    this.color,
    this.label,
    this.output,
    this.enabled = true,
    this.orientation = Axis.horizontal,
  }) : assert(value >= 0 && value <= 1);

  /// Which channel this slider edits; drives the track gradient.
  final HeroColorSliderChannel channel;

  /// Current channel fraction, 0..1 (hue/360, saturation, lightness, alpha).
  final double value;

  /// Called with the new channel fraction.
  final ValueChanged<double> onChanged;

  /// Current color used to build the gradient ([HSVColor.hue], saturation and
  /// value). Defaults to pure red at full saturation/value.
  final HSVColor? color;

  /// Optional label row (`.color-slider [data-slot="label"]`).
  final String? label;

  /// Optional output row (`.color-slider__output`, tabular figures).
  final String? output;

  final bool enabled;

  final Axis orientation;

  @override
  State<HeroColorSlider> createState() => _HeroColorSliderState();
}

class _HeroColorSliderState extends State<HeroColorSlider> {
  bool _dragging = false;

  HSVColor get _color => widget.color ?? _hsv(1, 0, 1, 1);

  KeyEventResult _handleKey(FocusNode node, KeyEvent event) {
    if (event is! KeyDownEvent || !widget.enabled) {
      return KeyEventResult.ignored;
    }
    final step = HardwareKeyboard.instance.isShiftPressed ? 0.1 : 0.01;
    var value = widget.value;
    switch (event.logicalKey) {
      case LogicalKeyboardKey.arrowLeft:
      case LogicalKeyboardKey.arrowDown:
        value -= step;
      case LogicalKeyboardKey.arrowRight:
      case LogicalKeyboardKey.arrowUp:
        value += step;
      default:
        return KeyEventResult.ignored;
    }
    widget.onChanged(value.clamp(0.0, 1.0));
    return KeyEventResult.handled;
  }

  @override
  Widget build(BuildContext context) {
    final horizontal = widget.orientation == Axis.horizontal;
    final header = [
      if (widget.label != null)
        Flexible(
          child: Text(
            widget.label!,
            style: TextStyle(
              fontSize: 14, // text-sm
              fontWeight: HeroTokens.weightMedium.resolve(context),
              color: HeroTokens.colorForeground.resolve(context),
            ),
          ),
        ),
      const Spacer(),
      if (widget.output != null)
        Text(
          widget.output!,
          style: TextStyle(
            fontSize: 14, // text-sm
            fontWeight: HeroTokens.weightMedium.resolve(context),
            color: HeroTokens.colorForeground.resolve(context),
            fontFeatures: const [FontFeature.tabularFigures()],
          ),
        ),
    ];

    final Widget body;
    if (horizontal) {
      body = Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          if (widget.label != null || widget.output != null) ...[
            Row(children: header),
            const SizedBox(height: 4), // gap-1
          ],
          SizedBox(height: 20, child: _buildTrack(context)),
        ],
      );
    } else {
      body = Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          if (widget.output != null)
            Text(
              widget.output!,
              textAlign: TextAlign.center,
              style: TextStyle(
                fontSize: 14,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorForeground.resolve(context),
                fontFeatures: const [FontFeature.tabularFigures()],
              ),
            ),
          if (widget.output != null || widget.label != null)
            const SizedBox(height: 8), // gap-2
          // h-full: fills 160px by default; wrap in a taller SizedBox for more.
          // w-5 track (20px) — the vertical stack of positioned children needs
          // a bounded width.
          SizedBox(
            width: 20,
            height: 160,
            child: _buildTrack(context),
          ),
          if (widget.output != null || widget.label != null)
            const SizedBox(height: 8),
          if (widget.label != null)
            Text(
              widget.label!,
              textAlign: TextAlign.center,
              style: TextStyle(
                fontSize: 14,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorForeground.resolve(context),
              ),
            ),
        ],
      );
    }

    if (!widget.enabled) {
      return Opacity(
        // status-disabled: opacity var(--disabled-opacity).
        opacity: HeroTokens.doubleDisabledOpacity.resolve(context),
        child: body,
      );
    }
    return body;
  }

  Widget _buildTrack(BuildContext context) {
    final horizontal = widget.orientation == Axis.horizontal;
    return LayoutBuilder(
      builder: (context, constraints) {
        final axisLength = horizontal
            ? constraints.maxWidth
            : constraints.maxHeight;
        // track = full length - 1.25rem (two 0.625rem caps).
        final trackLength = (axisLength - 20).clamp(0.0, axisLength);
        final thumbCenter = 10 + trackLength * widget.value;

        void updateFromLocal(Offset local) {
          if (trackLength <= 0) return;
          final fraction = horizontal
              ? (local.dx - 10) / trackLength
              : 1 - (local.dy - 10) / trackLength;
          widget.onChanged(fraction.clamp(0.0, 1.0));
        }

        final thumb = HeroFocusRing(
          radius: 12, // rounded-2xl thumb
          builder: (context, node, focused) => Focus(
            focusNode: node,
            onKeyEvent: _handleKey,
            child: Container(
              width: 16,
              height: 16,
              decoration: BoxDecoration(
                // `.color-slider__thumb`: border-3 border-white, shadow-overlay.
                borderRadius: BorderRadius.circular(12),
                border: Border.all(color: _heroSliderWhite, width: 3),
                boxShadow: [
                  for (final s in HeroTokens.shadowOverlay.resolve(context)) s,
                ],
              ),
            ),
          ),
        );

        return Stack(
          children: [
            Positioned.fill(
              child: CustomPaint(
                painter: _ColorSliderTrackPainter(
                  channel: widget.channel,
                  color: _color,
                  orientation: widget.orientation,
                ),
              ),
            ),
            // Topmost so panning that starts on the thumb is captured.
            Positioned.fill(
              child: GestureDetector(
                behavior: HitTestBehavior.opaque,
                onPanDown: widget.enabled
                    ? (d) {
                        setState(() => _dragging = true);
                        updateFromLocal(d.localPosition);
                      }
                    : null,
                onPanUpdate: widget.enabled
                    ? (d) => updateFromLocal(d.localPosition)
                    : null,
                onPanEnd: widget.enabled
                    ? (_) => setState(() => _dragging = false)
                    : null,
                onPanCancel: widget.enabled
                    ? () => setState(() => _dragging = false)
                    : null,
              ),
            ),
            AnimatedPositioned(
              // transform 250ms ease-out; instant while dragging.
              duration: _dragging
                  ? Duration.zero
                  : HeroMotion.durationOf(
                      context,
                      const Duration(milliseconds: 250),
                    ),
              curve: HeroMotion.out,
              left: horizontal
                  ? thumbCenter - 8
                  : (constraints.maxWidth - 16) / 2,
              top: horizontal
                  ? (constraints.maxHeight - 16) / 2
                  : 10 + trackLength * (1 - widget.value) - 8,
              child: thumb,
            ),
          ],
        );
      },
    );
  }
}

/// Paints the slider track: start cap (start color over the checkerboard),
/// gradient middle, end cap (end color), hairline inset shadows and the cap
/// corner radii (12 = `rounded-2xl`).
class _ColorSliderTrackPainter extends CustomPainter {
  const _ColorSliderTrackPainter({
    required this.channel,
    required this.color,
    required this.orientation,
  });

  final HeroColorSliderChannel channel;
  final HSVColor color;
  final Axis orientation;

  static const double _cap = 10; // 0.625rem edge cap

  List<Color> get _gradientColors => switch (channel) {
        HeroColorSliderChannel.hue => [
            _heroSliderHue(0),
            _heroSliderHue(60),
            _heroSliderHue(120),
            _heroSliderHue(180),
            _heroSliderHue(240),
            _heroSliderHue(300),
            _heroSliderHue(360),
          ],
        HeroColorSliderChannel.saturation => [
            _heroSliderWhite,
            _hsvColor(1, color.hue, 1, color.value),
          ],
        HeroColorSliderChannel.lightness => [
            _heroSliderBlack,
            _hsvColor(1, color.hue, 1, color.value),
            _heroSliderWhite,
          ],
        HeroColorSliderChannel.alpha => [
            _heroSliderWhite.withValues(alpha: 0),
            _hsvColor(1, color.hue, color.saturation, color.value),
          ],
      };

  @override
  void paint(Canvas canvas, Size size) {
    final horizontal = orientation == Axis.horizontal;
    final colors = _gradientColors;
    final startColor = colors.first;
    final endColor = colors.last;

    final mid = horizontal
        ? Rect.fromLTWH(_cap, 0, size.width - 2 * _cap, size.height)
        : Rect.fromLTWH(0, _cap, size.width, size.height - 2 * _cap);
    final startCap = horizontal
        ? Rect.fromLTWH(0, 0, _cap, size.height)
        : Rect.fromLTWH(0, size.height - _cap, size.width, _cap);
    final endCap = horizontal
        ? Rect.fromLTWH(size.width - _cap, 0, _cap, size.height)
        : Rect.fromLTWH(0, 0, size.width, _cap);

    // Start cap: checkerboard + start color overlay (`.color-slider__track
    // ::before`).
    final startR = horizontal
        ? RRect.fromRectAndCorners(
            startCap,
            topLeft: const Radius.circular(12),
            bottomLeft: const Radius.circular(12),
          )
        : RRect.fromRectAndCorners(
            startCap,
            bottomLeft: const Radius.circular(12),
            bottomRight: const Radius.circular(12),
          );
    _paintSegment(canvas, startR, startColor, checker: true);

    // Middle: checkerboard (alpha only) + channel gradient.
    if (channel == HeroColorSliderChannel.alpha) {
      _paintCheckerboard(canvas, mid);
    }
    final gradient = LinearGradient(
      begin: horizontal ? Alignment.centerLeft : Alignment.bottomCenter,
      end: horizontal ? Alignment.centerRight : Alignment.topCenter,
      colors: colors,
    ).createShader(mid);
    canvas.drawRect(mid, Paint()..shader = gradient);

    // End cap: plain end color (`.color-slider__track ::after`).
    final endR = horizontal
        ? RRect.fromRectAndCorners(
            endCap,
            topRight: const Radius.circular(12),
            bottomRight: const Radius.circular(12),
          )
        : RRect.fromRectAndCorners(
            endCap,
            topLeft: const Radius.circular(12),
            topRight: const Radius.circular(12),
          );
    canvas.drawRRect(endR, Paint()..color = endColor);

    // Inset hairline shadows: inset 0/±1px 0 0 rgba(0,0,0,0.1).
    final hairline = Paint()
      ..style = PaintingStyle.stroke
      ..strokeWidth = 1
      ..color = _heroSliderBlack.withValues(alpha: 0.1);
    canvas.drawRect(
      Rect.fromLTWH(0.5, 0.5, size.width - 1, size.height - 1),
      hairline,
    );
  }

  void _paintSegment(Canvas canvas, RRect rrect, Color color,
      {required bool checker}) {
    if (checker) {
      canvas.save();
      canvas.clipRRect(rrect);
      _paintCheckerboard(canvas, rrect.outerRect);
      canvas.restore();
    }
    canvas.drawRRect(rrect, Paint()..color = color);
  }

  void _paintCheckerboard(Canvas canvas, Rect rect) {
    const cell = 8.0;
    final paint = Paint();
    for (var y = rect.top; y < rect.bottom; y += cell) {
      for (var x = rect.left; x < rect.right; x += cell) {
        final isDark = ((x ~/ cell).toInt() + (y ~/ cell).toInt()).isOdd;
        paint.color = isDark ? _heroSliderCheckerDark : _heroSliderCheckerLight;
        canvas.drawRect(Rect.fromLTWH(x, y, cell, cell), paint);
      }
    }
  }

  @override
  bool shouldRepaint(_ColorSliderTrackPainter oldDelegate) =>
      oldDelegate.channel != channel ||
      oldDelegate.color != color ||
      oldDelegate.orientation != orientation;
}
