import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// HeroUI v3 color swatch shapes (color-swatch.css `.color-swatch--circle/square`).
enum HeroColorSwatchShape {
  /// Rounded tile (`.color-swatch--circle`) — the default.
  circle,

  /// Nearly-square tile (`.color-swatch--square`).
  square,
}

/// HeroUI v3 color swatch sizes (color-swatch.css `.color-swatch--xs..xl`).
enum HeroColorSwatchSize { xs, sm, md, lg, xl }

/// Payload colors of the swatch canvas (the color being displayed). These are
/// DATA, not chrome: the CSS builds the tile as
/// `linear-gradient(var(--color-swatch-current), ...)` over the transparency
/// checkerboard `repeating-conic-gradient(#efefef 0% 25%, #f7f7f7 0% 50%)`.
/// Constructed from HSV so no raw named or literal colors leak into
/// chrome roles.
Color _hsvColor(double alpha, double hue, double saturation, double value) =>
    HSVColor.fromAHSV(alpha, hue, saturation, value).toColor();

final Color _heroSwatchCheckerLight = _hsvColor(1, 0, 0, 0.969);
final Color _heroSwatchCheckerDark = _hsvColor(1, 0, 0, 0.937);
final Color _heroSwatchBlack = _hsvColor(1, 0, 0, 0);

/// Returns the tile side in logical px for a swatch size.
double heroColorSwatchSide(HeroColorSwatchSize size) => switch (size) {
      HeroColorSwatchSize.xs => 16,
      HeroColorSwatchSize.sm => 24,
      HeroColorSwatchSize.md => 32,
      HeroColorSwatchSize.lg => 36,
      HeroColorSwatchSize.xl => 40,
    };

/// Returns the corner radius for a swatch tile.
///
/// color-swatch.css: `--circle` is `rounded-2xl` at the default (16),
/// `rounded-lg`/`rounded-xl`/`rounded-3xl` for xs/sm/lg/xl; `--square` is
/// always `rounded-md` (6).
double heroColorSwatchRadius(
  HeroColorSwatchSize size,
  HeroColorSwatchShape shape,
) {
  if (shape == HeroColorSwatchShape.square) return 6;
  return switch (size) {
    HeroColorSwatchSize.xs => 8,
    HeroColorSwatchSize.sm => 12,
    HeroColorSwatchSize.md => 16,
    HeroColorSwatchSize.lg => 24,
    HeroColorSwatchSize.xl => 24,
  };
}

/// A HeroUI v3 color swatch (color-swatch.css) — a small rounded tile showing
/// a color over the transparency checkerboard.
///
/// ```dart
/// HeroColorSwatch(color: myColor, size: HeroColorSwatchSize.sm)
/// ```
class HeroColorSwatch extends StatelessWidget {
  const HeroColorSwatch({
    super.key,
    required this.color,
    this.shape = HeroColorSwatchShape.circle,
    this.size = HeroColorSwatchSize.md,
    this.selected = false,
    this.onTap,
    this.enabled = true,
  });

  /// The color to display. Null shows the bare transparency checkerboard.
  final Color? color;

  final HeroColorSwatchShape shape;

  final HeroColorSwatchSize size;

  /// Draws the selection ring (2px border in the swatch color + field
  /// shadow), mirroring the selected `color-swatch-picker__item`.
  final bool selected;

  /// When non-null the swatch behaves as a button and calls this on tap.
  final VoidCallback? onTap;

  final bool enabled;

  @override
  Widget build(BuildContext context) {
    final onTap = this.onTap;
    final side = heroColorSwatchSide(size);
    final radius = heroColorSwatchRadius(size, shape);
    final ringColor =
        color ?? HeroTokens.colorForeground.resolve(context);

    final tile = AnimatedContainer(
      duration: HeroMotion.durationOf(
        context,
        const Duration(milliseconds: 100),
      ),
      curve: HeroMotion.out,
      width: side,
      height: side,
      clipBehavior: Clip.antiAlias,
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(radius),
        // `.color-swatch` box-shadow: inset 0 0 0 1px rgba(0,0,0,0.1).
        border: Border.all(
          color: _heroSwatchBlack.withValues(alpha: 0.1),
          width: 1,
        ),
        boxShadow: selected
            ? [
                for (final s in HeroTokens.shadowField.resolve(context)) s,
              ]
            : null,
      ),
      foregroundDecoration: selected
          ? BoxDecoration(
              borderRadius: BorderRadius.circular(radius),
              // Selected item border: 2px in the swatch color.
              border: Border.all(color: ringColor, width: 2),
            )
          : null,
      child: CustomPaint(
        painter: _HeroSwatchPainter(color: color),
      ),
    );

    if (onTap == null) return tile;

    return HeroFocusRing(
      radius: radius,
      builder: (context, node, focused) => Focus(
        focusNode: node,
        onKeyEvent: (node, event) => _activateOnKey(event, onTap),
        child: GestureDetector(
          behavior: HitTestBehavior.opaque,
          onTap: enabled ? onTap : null,
          child: tile,
        ),
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

/// Paints the transparency checkerboard (data layer) then the color on top.
class _HeroSwatchPainter extends CustomPainter {
  const _HeroSwatchPainter({required this.color});

  final Color? color;

  @override
  void paint(Canvas canvas, Size size) {
    const cell = 8.0;
    final paint = Paint();
    for (var y = 0.0; y < size.height; y += cell) {
      for (var x = 0.0; x < size.width; x += cell) {
        final isDark = ((x ~/ cell).toInt() + (y ~/ cell).toInt()).isOdd;
        paint.color = isDark ? _heroSwatchCheckerDark : _heroSwatchCheckerLight;
        canvas.drawRect(Rect.fromLTWH(x, y, cell, cell), paint);
      }
    }
    final swatchColor = color;
    if (swatchColor != null) {
      canvas.drawRect(Offset.zero & size, Paint()..color = swatchColor);
    }
  }

  @override
  bool shouldRepaint(_HeroSwatchPainter oldDelegate) =>
      oldDelegate.color != color;
}
