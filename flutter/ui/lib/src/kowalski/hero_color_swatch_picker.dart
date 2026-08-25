import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// HeroUI v3 color swatch picker sizes (color-swatch-picker.css
/// `.color-swatch-picker--xs..xl`).
enum HeroColorSwatchPickerSize { xs, sm, md, lg, xl }

/// HeroUI v3 color swatch picker shapes.
enum HeroColorSwatchPickerShape {
  /// Round items (default; radius per size).
  circle,

  /// Squared items (`.color-swatch-picker--square`).
  square,
}

/// HeroUI v3 color swatch picker layouts.
enum HeroColorSwatchPickerLayout {
  /// Wrapping row (default; `flex flex-wrap gap-2`).
  grid,

  /// Vertical stack (`.color-swatch-picker--stack`).
  stack,
}

/// Payload colors of the swatch tiles + check indicator (the colors being
/// displayed). Constructed from HSV so no raw named or literal colors
/// leak into chrome roles.
Color _hsvColor(double alpha, double hue, double saturation, double value) =>
    HSVColor.fromAHSV(alpha, hue, saturation, value).toColor();

final Color _heroPickerCheckerLight = _hsvColor(1, 0, 0, 0.969);
final Color _heroPickerCheckerDark = _hsvColor(1, 0, 0, 0.937);
final Color _heroPickerCheckWhite = _hsvColor(1, 0, 0, 1);
final Color _heroPickerCheckBlack = _hsvColor(1, 0, 0, 0);
final Color _heroPickerTransparent = _hsvColor(0, 0, 0, 0);

({double side, double borderWidth, double itemRadius, double swatchRadius})
    _heroSwatchPickerGeometry(
  HeroColorSwatchPickerSize size,
  HeroColorSwatchPickerShape shape,
) {
  final side = switch (size) {
    HeroColorSwatchPickerSize.xs => 16.0,
    HeroColorSwatchPickerSize.sm => 24.0,
    HeroColorSwatchPickerSize.md => 32.0,
    HeroColorSwatchPickerSize.lg => 36.0,
    HeroColorSwatchPickerSize.xl => 40.0,
  };
  final borderWidth = switch (size) {
    HeroColorSwatchPickerSize.xs => 1.0,
    HeroColorSwatchPickerSize.sm => 2.0,
    HeroColorSwatchPickerSize.md => 2.0,
    HeroColorSwatchPickerSize.lg => 3.0,
    HeroColorSwatchPickerSize.xl => 3.0,
  };
  if (shape == HeroColorSwatchPickerShape.circle) {
    final radius = switch (size) {
      HeroColorSwatchPickerSize.xs => 8.0,
      HeroColorSwatchPickerSize.sm => 12.0,
      HeroColorSwatchPickerSize.md => 16.0,
      HeroColorSwatchPickerSize.lg => 24.0,
      HeroColorSwatchPickerSize.xl => 24.0,
    };
    return (
      side: side,
      borderWidth: borderWidth,
      itemRadius: radius,
      swatchRadius: radius,
    );
  }
  final itemRadius = switch (size) {
    HeroColorSwatchPickerSize.xs => 6.0,
    HeroColorSwatchPickerSize.sm => 8.0,
    HeroColorSwatchPickerSize.md => 12.0,
    HeroColorSwatchPickerSize.lg => 12.0,
    HeroColorSwatchPickerSize.xl => 12.0,
  };
  final swatchRadius = switch (size) {
    HeroColorSwatchPickerSize.xs => 6.0,
    HeroColorSwatchPickerSize.sm => 8.0,
    HeroColorSwatchPickerSize.md => 8.0,
    HeroColorSwatchPickerSize.lg => 8.0,
    HeroColorSwatchPickerSize.xl => 8.0,
  };
  return (
    side: side,
    borderWidth: borderWidth,
    itemRadius: itemRadius,
    swatchRadius: swatchRadius,
  );
}

/// A HeroUI v3 color swatch picker (color-swatch-picker.css) — a row/grid of
/// tappable color swatches with a check indicator on the selected one.
///
/// ```dart
/// HeroColorSwatchPicker(
///   colors: palette,
///   value: current,
///   onChanged: (c) => setState(() => current = c),
/// )
/// ```
class HeroColorSwatchPicker extends StatelessWidget {
  const HeroColorSwatchPicker({
    super.key,
    required this.colors,
    this.value,
    required this.onChanged,
    this.size = HeroColorSwatchPickerSize.md,
    this.shape = HeroColorSwatchPickerShape.circle,
    this.layout = HeroColorSwatchPickerLayout.grid,
    this.enabled = true,
  });

  /// The swatch colors to offer.
  final List<Color> colors;

  /// The currently selected color (compared by equality).
  final Color? value;

  final ValueChanged<Color> onChanged;

  final HeroColorSwatchPickerSize size;
  final HeroColorSwatchPickerShape shape;
  final HeroColorSwatchPickerLayout layout;
  final bool enabled;

  @override
  Widget build(BuildContext context) {
    final items = [
      for (final color in colors)
        _HeroSwatchPickerItem(
          color: color,
          selected: color == value,
          enabled: enabled,
          size: size,
          shape: shape,
          onTap: () => onChanged(color),
        ),
    ];

    final picker = switch (layout) {
      HeroColorSwatchPickerLayout.grid => Wrap(
          spacing: 8, // gap-2
          runSpacing: 8,
          children: items,
        ),
      HeroColorSwatchPickerLayout.stack => Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            for (final (i, item) in items.indexed) ...[
              if (i > 0) const SizedBox(height: 8), // gap-2
              item,
            ],
          ],
        ),
    };

    if (!enabled) {
      return Opacity(
        opacity: HeroTokens.doubleDisabledOpacity.resolve(context),
        child: picker,
      );
    }
    return picker;
  }
}

class _HeroSwatchPickerItem extends StatefulWidget {
  const _HeroSwatchPickerItem({
    required this.color,
    required this.selected,
    required this.enabled,
    required this.size,
    required this.shape,
    required this.onTap,
  });

  final Color color;
  final bool selected;
  final bool enabled;
  final HeroColorSwatchPickerSize size;
  final HeroColorSwatchPickerShape shape;
  final VoidCallback onTap;

  @override
  State<_HeroSwatchPickerItem> createState() => _HeroSwatchPickerItemState();
}

class _HeroSwatchPickerItemState extends State<_HeroSwatchPickerItem> {
  bool _hovered = false;

  @override
  Widget build(BuildContext context) {
    final (:side, :borderWidth, :itemRadius, :swatchRadius) =
        _heroSwatchPickerGeometry(widget.size, widget.shape);
    final fieldShadows = HeroTokens.shadowField.resolve(context);
    // data-light-color: dark check on light swatches.
    final lightColor = widget.color.computeLuminance() > 0.5;

    final swatch = ClipRRect(
      borderRadius: BorderRadius.circular(swatchRadius),
      child: CustomPaint(
        painter: _HeroSwatchPickerTilePainter(color: widget.color),
      ),
    );

    final item = AnimatedContainer(
      duration: HeroMotion.durationOf(
        context,
        const Duration(milliseconds: 100),
      ),
      curve: HeroMotion.out,
      width: side,
      height: side,
      decoration: BoxDecoration(
        borderRadius: BorderRadius.circular(itemRadius),
        // Selected: border-color = swatch color + field shadow; the swatch
        // scales down to 0.77 to reveal the white gap.
        border: Border.all(
          color: widget.selected ? widget.color : _heroPickerTransparent,
          width: borderWidth,
        ),
        boxShadow: widget.selected ? fieldShadows : null,
      ),
      child: Stack(
        children: [
          Positioned.fill(
            child: AnimatedScale(
              scale: widget.selected
                  ? 0.77
                  : (_hovered && widget.enabled ? 1.1 : 1.0),
              duration: HeroMotion.durationOf(
                context,
                const Duration(milliseconds: 100),
              ),
              curve: HeroMotion.out,
              child: swatch,
            ),
          ),
          if (widget.selected)
            Positioned.fill(
              child: AnimatedScale(
                scale: widget.selected ? 1.0 : 0.0,
                duration: HeroMotion.durationOf(
                  context,
                  const Duration(milliseconds: 150),
                ),
                curve: HeroMotion.out,
                child: Center(
                  child: Icon(
                    Icons.check_rounded,
                    size: side / 3, // size-1/3
                    color: lightColor
                        ? _heroPickerCheckBlack
                        : _heroPickerCheckWhite,
                  ),
                ),
              ),
            ),
        ],
      ),
    );

    return HeroFocusRing(
      radius: itemRadius,
      builder: (context, node, focused) => Focus(
        focusNode: node,
        onKeyEvent: (node, event) => _activateOnKey(event, widget.onTap),
        child: MouseRegion(
          onEnter: (_) => setState(() => _hovered = true),
          onExit: (_) => setState(() => _hovered = false),
          child: GestureDetector(
            behavior: HitTestBehavior.opaque,
            onTap: widget.enabled ? widget.onTap : null,
            child: item,
          ),
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

/// Paints the swatch tile: transparency checkerboard + the color on top.
class _HeroSwatchPickerTilePainter extends CustomPainter {
  const _HeroSwatchPickerTilePainter({required this.color});

  final Color color;

  @override
  void paint(Canvas canvas, Size size) {
    const cell = 8.0;
    final paint = Paint();
    for (var y = 0.0; y < size.height; y += cell) {
      for (var x = 0.0; x < size.width; x += cell) {
        final isDark = ((x ~/ cell).toInt() + (y ~/ cell).toInt()).isOdd;
        paint.color = isDark ? _heroPickerCheckerDark : _heroPickerCheckerLight;
        canvas.drawRect(Rect.fromLTWH(x, y, cell, cell), paint);
      }
    }
    canvas.drawRect(Offset.zero & size, Paint()..color = color);
  }

  @override
  bool shouldRepaint(_HeroSwatchPickerTilePainter oldDelegate) =>
      oldDelegate.color != color;
}
