import 'package:flutter/material.dart';
import 'package:mix/mix.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 popover placements (popover.css `[data-placement="*"]`).
enum HeroPopoverPlacement { top, right, bottom, left }

/// Cross-axis alignment of the popover relative to its anchor.
enum HeroPopoverAlignment { start, center, end }

/// The popover's arrow — a 12x12 lucide-style chevron (popover.js renders
/// `M0 0 C5.48 8 6.5 8 12 0 Z`), filled with the overlay surface color and
/// rotated to point at the anchor (`[data-placement]` arrow rotation).
class HeroPopoverArrow extends StatelessWidget {
  const HeroPopoverArrow({super.key});

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 12,
      height: 12,
      child: CustomPaint(
        painter: _HeroPopoverArrowPainter(
          HeroTokens.colorOverlay.resolve(context),
        ),
      ),
    );
  }
}

class _HeroPopoverArrowPainter extends CustomPainter {
  const _HeroPopoverArrowPainter(this.color);

  final Color color;

  @override
  void paint(Canvas canvas, Size size) {
    final path = Path()
      ..moveTo(0, 0)
      ..cubicTo(size.width * 0.457, size.height, size.width * 0.542,
          size.height, size.width, 0)
      ..close();
    canvas.drawPath(path, Paint()..color = color);
  }

  @override
  bool shouldRepaint(_HeroPopoverArrowPainter oldDelegate) =>
      color != oldDelegate.color;
}

/// The popover content chrome (popover.css `.popover__dialog` — `p-4
/// outline-none`; `.popover__heading` — `font-medium`).
class HeroPopoverDialog extends StatelessWidget {
  const HeroPopoverDialog({
    super.key,
    this.title,
    this.child,
  });

  final String? title;
  final Widget? child;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.all(16), // p-4
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          if (title != null) ...[
            // SizedBox(width: infinity): stretches just the title to the
            // popover's bounded width (see `_HeroPopoverRoute`'s
            // ConstrainedBox) so the Text has something real to wrap/
            // ellipsize against, without forcing `child` (e.g. a calendar
            // with its own fixed-width/tap layout) to stretch too.
            SizedBox(
              width: double.infinity,
              child: Text(
                title!,
                overflow: TextOverflow.ellipsis,
                softWrap: true,
                style: TextStyle(
                  fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                  fontWeight: HeroTokens.weightMedium.resolve(context),
                  color: HeroTokens.colorForeground.resolve(context),
                ),
              ),
            ),
            const SizedBox(height: 8),
          ],
          if (child != null) child!,
        ],
      ),
    );
  }
}

/// A HeroUI v3 popover (popover.css) — an anchored overlay with an arrow,
/// `bg-overlay shadow-overlay`, radius `min(32px, var(--radius-3xl))` = 24.
///
/// The overlay fades in with a 90%→100% scale (150ms `--ease-smooth`) and a
/// 4px slide from the placement direction; the dialog chrome is
/// [HeroPopoverDialog].
class HeroPopover extends StatefulWidget {
  const HeroPopover({
    super.key,
    required this.trigger,
    required this.content,
    this.placement = HeroPopoverPlacement.bottom,
    this.alignment = HeroPopoverAlignment.center,
    this.showArrow = true,
    this.gap = 4,
  });

  /// Builds the anchor widget (`.popover__trigger` — inline-block,
  /// interactive cursor). Wire its press handler to [VoidCallback] to open
  /// the popover (keeps the trigger's own tap handling intact).
  final Widget Function(VoidCallback open) trigger;

  /// The popover body.
  final Widget content;

  final HeroPopoverPlacement placement;
  final HeroPopoverAlignment alignment;
  final bool showArrow;

  /// Resting gap between the anchor and the popover edge (px).
  final double gap;

  @override
  State<HeroPopover> createState() => _HeroPopoverState();
}

class _HeroPopoverState extends State<HeroPopover> {
  final GlobalKey _triggerKey = GlobalKey();

  void _open() {
    final box =
        _triggerKey.currentContext?.findRenderObject() as RenderBox?;
    if (box == null || !box.hasSize) return;
    final anchor = box.localToGlobal(Offset.zero) & box.size;
    showHeroPopover<void>(
      context,
      anchor: anchor,
      placement: widget.placement,
      alignment: widget.alignment,
      showArrow: widget.showArrow,
      gap: widget.gap,
      builder: (context) => widget.content,
    );
  }

  @override
  Widget build(BuildContext context) {
    return KeyedSubtree(
      key: _triggerKey,
      child: widget.trigger(_open),
    );
  }
}

/// Shows a HeroUI v3 popover anchored to [anchor] (a rect in global
/// coordinates).
///
/// The route wraps its content in the ambient [MixScope] and positions the
/// popover against [anchor], flipping to the opposite edge when the chosen
/// placement would overflow the screen.
Future<T?> showHeroPopover<T>(
  BuildContext context, {
  required Rect anchor,
  required WidgetBuilder builder,
  HeroPopoverPlacement placement = HeroPopoverPlacement.bottom,
  HeroPopoverAlignment alignment = HeroPopoverAlignment.center,
  bool showArrow = true,
  double gap = 4,
  double? maxWidth,
  double? minWidth,
  bool barrierDismissible = true,
  String? barrierLabel,
}) {
  final scope = MixScope.of(context);

  return showGeneralDialog<T>(
    context: context,
    // showGeneralDialog defaults useRootNavigator to true, which escapes the
    // caller's own Navigator — e.g. a widgetbook use case's isolated preview
    // sandbox (where the Theme addon installs its own HeroScope) — onto the
    // outermost one. Landing there picks up THAT navigator's ambient
    // DefaultTextStyle instead of HeroScope's (decoration: none, correct
    // colors), which is why unstyled popover content (a bare Text with no
    // explicit TextStyle) can render with a foreign color/underline. Staying
    // on the caller's own Navigator keeps it under the right HeroScope, same
    // as showHeroAnchoredOverlay's Overlay.maybeOf(context) does.
    useRootNavigator: false,
    barrierColor: HeroTokens.colorTransparent.resolve(context),
    barrierDismissible: barrierDismissible,
    barrierLabel: barrierLabel ?? 'Dismiss popover',
    transitionDuration: const Duration(milliseconds: 150),
    // The popover drives its own fade/scale/slide from the route animation.
    transitionBuilder:
        (context, animation, secondaryAnimation, child) => child,
    pageBuilder: (context, animation, secondaryAnimation) {
      return MixScope(
        tokens: scope.tokens,
        orderOfModifiers: scope.orderOfModifiers,
        // Belt-and-braces (see HeroScope/HeroPopoverPanel): don't rely solely
        // on useRootNavigator to keep foreign decoration/color out.
        child: DefaultTextStyle.merge(
          style: const TextStyle(decoration: TextDecoration.none),
          child: _HeroPopoverRoute(
            anchor: anchor,
            placement: placement,
            alignment: alignment,
            showArrow: showArrow,
            gap: gap,
            maxWidth: maxWidth,
            minWidth: minWidth,
            animation: animation,
            child: builder(context),
          ),
        ),
      );
    },
  );
}

class _HeroPopoverRoute extends StatelessWidget {
  const _HeroPopoverRoute({
    required this.anchor,
    required this.placement,
    required this.alignment,
    required this.showArrow,
    required this.gap,
    required this.animation,
    required this.child,
    this.maxWidth,
    this.minWidth,
  });

  final Rect anchor;
  final HeroPopoverPlacement placement;
  final HeroPopoverAlignment alignment;
  final bool showArrow;
  final double gap;
  final Animation<double> animation;
  final Widget child;
  final double? maxWidth;
  final double? minWidth;

  @override
  Widget build(BuildContext context) {
    final eased = CurvedAnimation(
      parent: animation,
      curve: heroEaseSmooth,
      reverseCurve: heroEaseSmooth,
    );

    // Enter: `animate-in duration-150 ease-smooth fade-in-0 zoom-in-90`
    // plus a 4px slide from the placement side; the reverse handles the
    // 100ms `zoom-out-95 fade-out` exit (single route controller).
    final slide = switch (placement) {
      HeroPopoverPlacement.bottom => const Offset(0, -4),
      HeroPopoverPlacement.top => const Offset(0, 4),
      HeroPopoverPlacement.left => const Offset(4, 0),
      HeroPopoverPlacement.right => const Offset(-4, 0),
    };

    // Popovers sit in a Positioned with no forced size, so without a cap
    // here a Text child just stretches to the screen edge instead of
    // wrapping/ellipsizing — default to a sane popover width (HeroUI
    // popovers aren't meant to span the viewport).
    var content = ConstrainedBox(
      constraints: BoxConstraints(
        minWidth: minWidth ?? 0,
        maxWidth: maxWidth ?? 320,
      ),
      child: child,
    );

    final popover = _HeroPopoverSurface(
      child: DefaultTextStyle.merge(
        style: TextStyle(
          fontSize: HeroTokens.typeSm.resolve(context).fontSize,
        ),
        child: content,
      ),
    );

    return LayoutBuilder(
      builder: (context, constraints) {
        return _HeroPopoverPositioned(
          anchor: anchor,
          placement: placement,
          alignment: alignment,
          showArrow: showArrow,
          gap: gap,
          screen: constraints.biggest,
          child: AnimatedBuilder(
            animation: eased,
            child: popover,
            builder: (context, child) {
              final t = eased.value;
              return Opacity(
                opacity: t,
                child: Transform.translate(
                  offset: Offset.lerp(slide, Offset.zero, t)!,
                  child: Transform.scale(
                    scale: 0.9 + 0.1 * t,
                    child: child,
                  ),
                ),
              );
            },
          ),
        );
      },
    );
  }
}

/// The popover surface — `bg-overlay shadow-overlay`, radius 24.
class _HeroPopoverSurface extends StatelessWidget {
  const _HeroPopoverSurface({required this.child});

  final Widget child;

  @override
  Widget build(BuildContext context) {
    final shadows = HeroTokens.shadowOverlay.resolve(context);
    return Container(
      decoration: BoxDecoration(
        color: HeroTokens.colorOverlay.resolve(context),
        borderRadius: BorderRadius.circular(
          HeroTokens.radius3xl.resolve(context).x,
        ),
        boxShadow: [for (final s in shadows) s],
      ),
      child: child,
    );
  }
}

/// Positions the popover next to [anchor] and clamps it to the screen,
/// flipping the placement when there is no room.
class _HeroPopoverPositioned extends StatefulWidget {
  const _HeroPopoverPositioned({
    required this.anchor,
    required this.placement,
    required this.alignment,
    required this.showArrow,
    required this.gap,
    required this.screen,
    required this.child,
  });

  final Rect anchor;
  final HeroPopoverPlacement placement;
  final HeroPopoverAlignment alignment;
  final bool showArrow;
  final double gap;
  final Size screen;
  final Widget child;

  @override
  State<_HeroPopoverPositioned> createState() => _HeroPopoverPositionedState();
}

class _HeroPopoverPositionedState extends State<_HeroPopoverPositioned> {
  Size? _size;

  @override
  Widget build(BuildContext context) {
    final size = _size ?? Size.zero;
    final screen = widget.screen;
    final anchor = widget.anchor;
    const margin = 8.0;
    const arrow = 6.0;

    // Preferred placement may flip when the anchored edge has no room.
    var placement = widget.placement;
    var overflow = switch (placement) {
      HeroPopoverPlacement.bottom => anchor.bottom + widget.gap + (size.height == 0 ? 0 : size.height) > screen.height - margin,
      HeroPopoverPlacement.top => anchor.top - widget.gap - (size.height == 0 ? 0 : size.height) < margin,
      HeroPopoverPlacement.left => anchor.left - widget.gap - (size.width == 0 ? 0 : size.width) < margin,
      HeroPopoverPlacement.right => anchor.right + widget.gap + (size.width == 0 ? 0 : size.width) > screen.width - margin,
    };
    if (overflow) {
      placement = switch (placement) {
        HeroPopoverPlacement.bottom => HeroPopoverPlacement.top,
        HeroPopoverPlacement.top => HeroPopoverPlacement.bottom,
        HeroPopoverPlacement.left => HeroPopoverPlacement.right,
        HeroPopoverPlacement.right => HeroPopoverPlacement.left,
      };
    }

    // Slot the popover against the anchor, then clamp the cross axis.
    final (slotDx, slotDy) = switch (placement) {
      HeroPopoverPlacement.bottom => (
          _crossAnchor(anchor, size, screen, widget.alignment, isHorizontal: true),
          anchor.bottom + widget.gap,
        ),
      HeroPopoverPlacement.top => (
          _crossAnchor(anchor, size, screen, widget.alignment, isHorizontal: true),
          anchor.top - widget.gap - size.height,
        ),
      HeroPopoverPlacement.left => (
          anchor.left - widget.gap - size.width,
          _crossAnchor(anchor, size, screen, widget.alignment, isHorizontal: false),
        ),
      HeroPopoverPlacement.right => (
          anchor.right + widget.gap,
          _crossAnchor(anchor, size, screen, widget.alignment, isHorizontal: false),
        ),
    };

    final left = (placement == HeroPopoverPlacement.left ||
            placement == HeroPopoverPlacement.right)
        ? slotDx
        : _clampCross(slotDx, size.width, screen.width, margin, isHorizontal: true);
    final top = (placement == HeroPopoverPlacement.bottom ||
            placement == HeroPopoverPlacement.top)
        ? slotDy
        : _clampCross(slotDy, size.height, screen.height, margin, isHorizontal: false);

    // Never let the popover run off either edge (arrow overhang included).
    // Guard lower <= upper: a full-width trigger (minWidth == anchor.width ==
    // screen width) makes the upper bound fall below the lower one, and
    // clamp() would throw "Invalid argument(s): 2.0". Pin to the margin then.
    final maxLeft = screen.width - size.width - margin + arrow;
    final maxTop = screen.height - size.height - margin + arrow;
    final finalLeft = maxLeft < margin - arrow
        ? (margin - arrow).toDouble()
        : left.clamp(margin - arrow, maxLeft).toDouble();
    final finalTop = maxTop < margin - arrow
        ? (margin - arrow).toDouble()
        : top.clamp(margin - arrow, maxTop).toDouble();

    final stackChildren = <Widget>[
      Positioned(
        left: finalLeft,
        top: finalTop,
        child: _HeroPopoverMeasure(
          onSize: (s) {
            if (s != _size) setState(() => _size = s);
          },
          child: widget.child,
        ),
      ),
    ];

    // The arrow is drawn at the anchored edge of the measured popover,
    // centered on the anchor along the cross axis and protruding 6px
    // (popover.css arrow rotation: bottom 180deg, left -90deg, right 90deg).
    if (widget.showArrow && size != Size.zero) {
      // Uses the (possibly flipped) `placement` so the arrow follows the
      // popover to the opposite side.
      final (arrowDx, arrowDy, rotation) = switch (placement) {
        HeroPopoverPlacement.bottom => (
            _clampCenter(anchor.center.dx - finalLeft, size.width) - 6,
            -8.0,
            3.141592653589793,
          ),
        HeroPopoverPlacement.top => (
            _clampCenter(anchor.center.dx - finalLeft, size.width) - 6,
            size.height - 4,
            0.0,
          ),
        HeroPopoverPlacement.left => (
            size.width - 4,
            _clampCenter(anchor.center.dy - finalTop, size.height) - 6,
            -1.5707963267948966,
          ),
        HeroPopoverPlacement.right => (
            -8.0,
            _clampCenter(anchor.center.dy - finalTop, size.height) - 6,
            1.5707963267948966,
          ),
      };
      stackChildren.add(
        Positioned(
          left: finalLeft + arrowDx,
          top: finalTop + arrowDy,
          child: Transform.rotate(
            angle: rotation,
            child: const HeroPopoverArrow(),
          ),
        ),
      );
    }

    return Stack(children: stackChildren);
  }

  /// Centers [value] inside [extent] with a 6px margin for the 12px arrow.
  double _clampCenter(double value, double extent) {
    const minC = 6.0;
    final maxC = extent - 6.0 < minC ? minC : extent - 6.0;
    return value.clamp(minC, maxC).toDouble();
  }

  double _crossAnchor(
    Rect anchor,
    Size size,
    Size screen,
    HeroPopoverAlignment alignment, {
    required bool isHorizontal,
  }) {
    return switch (alignment) {
      HeroPopoverAlignment.start => isHorizontal ? anchor.left : anchor.top,
      HeroPopoverAlignment.center =>
        isHorizontal ? anchor.center.dx - size.width / 2 : anchor.center.dy - size.height / 2,
      HeroPopoverAlignment.end =>
        isHorizontal ? anchor.right - size.width : anchor.bottom - size.height,
    };
  }

  double _clampCross(
    double value,
    double extent,
    double screenExtent,
    double margin, {
    required bool isHorizontal,
  }) {
    if (extent + 2 * margin > screenExtent) return margin;
    return value.clamp(margin, screenExtent - extent - margin).toDouble();
  }
}

class _HeroPopoverMeasure extends StatefulWidget {
  const _HeroPopoverMeasure({required this.onSize, required this.child});

  final ValueChanged<Size> onSize;
  final Widget child;

  @override
  State<_HeroPopoverMeasure> createState() => _HeroPopoverMeasureState();
}

class _HeroPopoverMeasureState extends State<_HeroPopoverMeasure> {
  final GlobalKey _key = GlobalKey();

  @override
  Widget build(BuildContext context) {
    // Re-measure after every build so layout changes (text scale, content
    // swaps) reposition the popover.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      final box = _key.currentContext?.findRenderObject() as RenderBox?;
      if (box != null && box.hasSize) {
        widget.onSize(box.size);
      }
    });
    return KeyedSubtree(key: _key, child: widget.child);
  }
}
