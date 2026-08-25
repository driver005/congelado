import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../foundation/hero_motion.dart';
import 'hero_focus_ring.dart';

/// Payload colors of the color-area canvas (the HSV face being displayed).
/// Data, not chrome — the CSS gets them from JS
/// (`--color-area-background` / `--color-area-thumb-color`); constructed from
/// HSV so raw named or literal colors never leak into chrome roles.
Color _hsvColor(double alpha, double hue, double saturation, double value) =>
    HSVColor.fromAHSV(alpha, hue, saturation, value).toColor();

HSVColor _hsv(double alpha, double hue, double saturation, double value) =>
    HSVColor.fromAHSV(alpha, hue, saturation, value);

final Color _heroAreaWhite = _hsvColor(1, 0, 0, 1);
final Color _heroAreaBlack = _hsvColor(1, 0, 0, 0);

/// HeroUI v3 color area (color-area.css) — a 2D saturation × value square.
///
/// The area shows the full SV face of the current hue; dragging the thumb
/// reports new saturation/value while keeping hue and alpha. Mirror of
/// `.color-area` (`max-w-56` 224px, square, `rounded-2xl` 16, inner 1px
/// shadow) with the `.color-area__thumb` (16px, 20px while dragging,
/// 3px white border, `status-focused` ring).
class HeroColorArea extends StatefulWidget {
  const HeroColorArea({
    super.key,
    this.value,
    required this.onChanged,
    this.enabled = true,
    this.showDots = false,
  });

  /// The current color. Null renders the hue-0 face at full saturation/value.
  /// Only [HSVColor.saturation] and [HSVColor.value] are editable here.
  final HSVColor? value;

  /// Called with the new color (same hue/alpha, new saturation/value).
  final ValueChanged<HSVColor> onChanged;

  final bool enabled;

  /// Dot-pattern overlay (`.color-area--show-dots`).
  final bool showDots;

  @override
  State<HeroColorArea> createState() => _HeroColorAreaState();
}

class _HeroColorAreaState extends State<HeroColorArea> {
  bool _dragging = false;

  HSVColor get _color => widget.value ?? _hsv(1, 0, 1, 1);

  void _updateFromLocal(Offset local, Size size) {
    if (!widget.enabled || size.width <= 0 || size.height <= 0) return;
    final saturation = (local.dx / size.width).clamp(0.0, 1.0);
    final value = 1.0 - (local.dy / size.height).clamp(0.0, 1.0);
    final current = _color;
    widget.onChanged(
      HSVColor.fromAHSV(current.alpha, current.hue, saturation, value),
    );
  }

  KeyEventResult _handleKey(FocusNode node, KeyEvent event) {
    if (event is! KeyDownEvent || !widget.enabled) {
      return KeyEventResult.ignored;
    }
    final step = HardwareKeyboard.instance.isShiftPressed ? 0.1 : 0.01;
    var (saturation, value) = (_color.saturation, _color.value);
    switch (event.logicalKey) {
      case LogicalKeyboardKey.arrowLeft:
        saturation -= step;
      case LogicalKeyboardKey.arrowRight:
        saturation += step;
      case LogicalKeyboardKey.arrowUp:
        value += step;
      case LogicalKeyboardKey.arrowDown:
        value -= step;
      default:
        return KeyEventResult.ignored;
    }
    final current = _color;
    widget.onChanged(
      HSVColor.fromAHSV(
        current.alpha,
        current.hue,
        saturation.clamp(0.0, 1.0),
        value.clamp(0.0, 1.0),
      ),
    );
    return KeyEventResult.handled;
  }

  @override
  Widget build(BuildContext context) {
    final color = _color;
    return ConstrainedBox(
      // `.color-area`: w-full max-w-56 (224) shrink-0.
      constraints: const BoxConstraints(maxWidth: 224),
      child: AspectRatio(
        aspectRatio: 1,
        child: LayoutBuilder(
          builder: (context, constraints) {
            final size = Size(constraints.maxWidth, constraints.maxHeight);
            return Stack(
              fit: StackFit.expand,
              children: [
                // `.color-area` background + inner 1px shadow, rounded-2xl.
                ClipRRect(
                  borderRadius: BorderRadius.circular(16),
                  child: CustomPaint(
                    painter: _ColorAreaPainter(
                      hue: color.hue,
                      showDots: widget.showDots,
                    ),
                  ),
                ),
                // Drag anywhere on the area (topmost so panning that starts
                // on the thumb is also captured).
                GestureDetector(
                  behavior: HitTestBehavior.opaque,
                  onPanDown: widget.enabled
                      ? (d) {
                          setState(() => _dragging = true);
                          _updateFromLocal(d.localPosition, size);
                        }
                      : null,
                  onPanUpdate: widget.enabled
                      ? (d) => _updateFromLocal(d.localPosition, size)
                      : null,
                  onPanEnd: widget.enabled
                      ? (_) => setState(() => _dragging = false)
                      : null,
                  onPanCancel: widget.enabled
                      ? () => setState(() => _dragging = false)
                      : null,
                ),
                // `.color-area__thumb` — positioned at (saturation, 1-value).
                Align(
                  alignment: FractionalOffset(
                    color.saturation,
                    1 - color.value,
                  ),
                  child: HeroFocusRing(
                    radius: 12, // rounded-xl thumb
                    builder: (context, node, focused) => Focus(
                      focusNode: node,
                      onKeyEvent: _handleKey,
                      child: AnimatedContainer(
                        // size-4 -> size-5 while dragging, 150ms ease-out.
                        duration: HeroMotion.durationOf(
                          context,
                          const Duration(milliseconds: 150),
                        ),
                        curve: HeroMotion.out,
                        width: _dragging ? 20 : 16,
                        height: _dragging ? 20 : 16,
                        decoration: BoxDecoration(
                          color: _hsvColor(
                            1,
                            color.hue,
                            color.saturation,
                            color.value,
                          ),
                          borderRadius: BorderRadius.circular(12),
                          border: Border.all(color: _heroAreaWhite, width: 3),
                          boxShadow: [
                            // 0 0 0 1px rgba(0,0,0,0.1)
                            BoxShadow(
                              color: _heroAreaBlack.withValues(alpha: 0.1),
                              blurRadius: 0,
                              spreadRadius: 1,
                            ),
                          ],
                        ),
                        // inset 0 0 0 1px rgba(0,0,0,0.1)
                        foregroundDecoration: BoxDecoration(
                          borderRadius: BorderRadius.circular(12),
                          border: Border.all(
                            color: _heroAreaBlack.withValues(alpha: 0.1),
                            width: 1,
                          ),
                        ),
                      ),
                    ),
                  ),
                ),
              ],
            );
          },
        ),
      ),
    );
  }
}

/// Paints the HSV saturation/value face: base hue color, a horizontal white
/// gradient (saturation axis) and a vertical black gradient (value axis),
/// then the optional dot overlay and the inner hairline shadow.
class _ColorAreaPainter extends CustomPainter {
  const _ColorAreaPainter({required this.hue, required this.showDots});

  final double hue;
  final bool showDots;

  @override
  void paint(Canvas canvas, Size size) {
    final rect = Offset.zero & size;
    final base = _hsvColor(1, hue, 1, 1);

    canvas.drawRect(rect, Paint()..color = base);
    // Saturation: white (left) -> transparent (right).
    canvas.drawRect(
      rect,
      Paint()
        ..shader = LinearGradient(
          begin: Alignment.centerLeft,
          end: Alignment.centerRight,
          colors: [
            _heroAreaWhite,
            _heroAreaWhite.withValues(alpha: 0),
          ],
        ).createShader(rect),
    );
    // Value: transparent (top) -> black (bottom).
    canvas.drawRect(
      rect,
      Paint()
        ..shader = LinearGradient(
          begin: Alignment.topCenter,
          end: Alignment.bottomCenter,
          colors: [
            _heroAreaBlack.withValues(alpha: 0),
            _heroAreaBlack,
          ],
        ).createShader(rect),
    );

    if (showDots) {
      // radial-gradient(circle, rgba(255,255,255,0.2) 1px, transparent 1px),
      // background-size 8px 8px, positioned at 50%.
      final dot = Paint()..color = _heroAreaWhite.withValues(alpha: 0.2);
      for (var x = 4.0; x < size.width; x += 8) {
        for (var y = 4.0; y < size.height; y += 8) {
          canvas.drawCircle(Offset(x, y), 1, dot);
        }
      }
    }

    // box-shadow: inset 0 0 0 1px rgba(0,0,0,0.1).
    canvas.drawRRect(
      RRect.fromRectAndRadius(rect.deflate(0.5), const Radius.circular(16)),
      Paint()
        ..style = PaintingStyle.stroke
        ..strokeWidth = 1
        ..color = _heroAreaBlack.withValues(alpha: 0.1),
    );
  }

  @override
  bool shouldRepaint(_ColorAreaPainter oldDelegate) =>
      oldDelegate.hue != hue || oldDelegate.showDots != showDots;
}
