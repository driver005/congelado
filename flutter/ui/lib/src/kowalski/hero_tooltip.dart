import 'dart:async';

import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 tooltip placements (tooltip.css `[data-placement=...]`,
/// react-aria Tooltip `placement`).
///
/// `top`/`bottom`/`left`/`right` are fixed sides; `start`/`end` resolve
/// against the ambient writing direction (start == left in LTR, right in
/// RTL).
enum HeroTooltipPlacement { top, right, bottom, left, start, end }

/// A HeroUI v3 tooltip (tooltip.css `.tooltip`) — a small overlay shown
/// while the trigger is hovered (or focused), placed on the requested side.
///
/// tooltip.css: `.tooltip` — `max-w-xs` (320), `bg-overlay`, `p-2` (8),
/// `text-xs`, radius `min(32px, var(--radius-xl))` = 12, `shadow-overlay`.
/// Show/hide are instant (no animation); the tooltip stays visible only
/// while the pointer is over the trigger. The open delay defaults to 300ms;
/// `--tooltip-close-delay` (500ms) is available via `closeDelay` (default 0
/// = hide as soon as the pointer leaves).
///
/// The arrow (`.tooltip [data-slot="overlay-arrow"]`) is the HeroUI 12×12
/// chevron — `M0 0 C5.48483 8 6.5 8 12 0 Z`, `fill: var(--overlay)`,
/// `stroke: border/40` — rotated per placement (top 0°, bottom 180°, left
/// -90°, right 90°). HeroUI's offset is `7` with an arrow, `3` without.
class HeroTooltip extends StatefulWidget {
  const HeroTooltip({
    super.key,
    required this.message,
    required this.child,
    this.placement = HeroTooltipPlacement.top,
    this.offset,
    this.showArrow = false,
    this.delay,
    this.closeDelay,
    this.enabled = true,
    this.maxWidth,
  });

  /// The tooltip text (`text-xs`, `max-w-xs`).
  final String message;

  /// The widget the tooltip is attached to (`.tooltip__trigger`).
  final Widget child;

  /// Which side of the trigger the tooltip appears on.
  final HeroTooltipPlacement placement;

  /// Gap between the trigger and the tooltip/arrow tip. Defaults to HeroUI's
  /// `7` when [showArrow] is true, `3` otherwise.
  final double? offset;

  /// Renders the HeroUI chevron arrow pointing at the trigger.
  final bool showArrow;

  /// Delay before the tooltip appears (default 300ms).
  final Duration? delay;

  /// Delay before the tooltip hides after the pointer leaves (default 0 —
  /// the tooltip is visible only while the trigger is hovered/focused).
  final Duration? closeDelay;

  /// When false the tooltip never shows.
  final bool enabled;

  /// Optional max width (default `max-w-xs` 320).
  final double? maxWidth;

  @override
  State<HeroTooltip> createState() => _HeroTooltipState();
}

class _HeroTooltipState extends State<HeroTooltip> {
  final GlobalKey _targetKey = GlobalKey();
  final FocusNode _focusNode = FocusNode();
  OverlayEntry? _entry;
  Timer? _openTimer;
  Timer? _closeTimer;
  HeroTooltipPlacement _placement = HeroTooltipPlacement.top;
  Rect? _anchorRect;

  // Resolved at the TRIGGER's build (the HeroScope that themes the trigger),
  // then passed into the overlay entry as concrete values — the navigator
  // Overlay can sit above the use case's HeroScope (e.g. a theme addon), so
  // resolving inside the entry would pick the wrong theme.
  late List<BoxShadow> _shadow;
  late Color _overlayColor;
  late Color _overlayForeground;
  late Color _arrowStroke;

  Duration get _delay => widget.delay ?? const Duration(milliseconds: 300);
  Duration get _closeDelay => widget.closeDelay ?? Duration.zero;

  /// The resolved side (start/end folded against the writing direction).
  HeroTooltipPlacement get _resolvedPlacement {
    final ltr = Directionality.of(context) == TextDirection.ltr;
    return switch (widget.placement) {
      HeroTooltipPlacement.start => ltr
          ? HeroTooltipPlacement.left
          : HeroTooltipPlacement.right,
      HeroTooltipPlacement.end => ltr
          ? HeroTooltipPlacement.right
          : HeroTooltipPlacement.left,
      HeroTooltipPlacement.top ||
      HeroTooltipPlacement.right ||
      HeroTooltipPlacement.bottom ||
      HeroTooltipPlacement.left =>
          widget.placement,
    };
  }

  @override
  void initState() {
    super.initState();
    _focusNode.addListener(_onFocusChanged);
  }

  @override
  void dispose() {
    _focusNode.removeListener(_onFocusChanged);
    _focusNode.dispose();
    _openTimer?.cancel();
    _closeTimer?.cancel();
    _entry?.remove();
    super.dispose();
  }

  void _onFocusChanged() {
    if (_focusNode.hasFocus) {
      _scheduleOpen();
    } else {
      _scheduleClose();
    }
  }

  void _scheduleOpen() {
    if (!widget.enabled) return;
    _closeTimer?.cancel();
    _openTimer?.cancel();
    _openTimer = Timer(_delay, _show);
  }

  void _scheduleClose() {
    _openTimer?.cancel();
    _closeTimer?.cancel();
    if (_entry == null) return;
    _closeTimer = Timer(_closeDelay, _hide);
  }

  void _show() {
    if (_entry != null || !mounted) return;
    final overlay = Overlay.maybeOf(context);
    final targetBox =
        _targetKey.currentContext?.findRenderObject() as RenderBox?;
    final overlayBox = overlay?.context.findRenderObject() as RenderBox?;
    if (overlay == null ||
        targetBox == null ||
        overlayBox == null ||
        !targetBox.attached) {
      return;
    }
    _placement = _resolvedPlacement;
    // Measure the trigger in the OVERLAY's coordinate space: the nearest
    // Overlay may be a nested one (e.g. the widgetbook DeviceFrameAddon
    // wraps use cases in their own Navigator), whose origin/scale differs
    // from global coordinates.
    final topLeft = overlayBox.globalToLocal(
      targetBox.localToGlobal(Offset.zero),
    );
    _anchorRect = topLeft & targetBox.size;
    _entry = OverlayEntry(builder: _buildOverlay);
    overlay.insert(_entry!);
  }

  void _hide() {
    _removeEntry();
  }

  void _removeEntry() {
    final entry = _entry;
    _entry = null;
    if (entry != null && entry.mounted) {
      entry.remove();
    }
  }

  double get _gap => widget.offset ?? (widget.showArrow ? 7.0 : 3.0);

  @override
  Widget build(BuildContext context) {
    // Resolve the tooltip's theme at the trigger.
    _shadow = HeroTokens.shadowOverlay.resolve(context);
    _overlayColor = HeroTokens.colorOverlay.resolve(context);
    _overlayForeground = HeroTokens.colorOverlayForeground.resolve(context);
    _arrowStroke =
        HeroTokens.colorBorder.resolve(context).withValues(alpha: 0.4);

    return Focus(
      focusNode: _focusNode,
      child: MouseRegion(
        key: _targetKey,
        onEnter: widget.enabled ? (_) => _scheduleOpen() : null,
        onExit: widget.enabled ? (_) => _scheduleClose() : null,
        child: widget.child,
      ),
    );
  }

  Widget _buildOverlay(BuildContext context) {
    // The Overlay lays entries out with BoxConstraints.tight(screen size);
    // the CustomSingleChildLayout gives the panel loose constraints (so it
    // sizes to its content) and positions it from the measured anchor rect.
    return CustomSingleChildLayout(
      delegate: _TooltipPositionDelegate(
        anchorRect: _anchorRect ?? Rect.zero,
        placement: _placement,
        gap: _gap,
      ),
      child: _TooltipPanel(
        message: widget.message,
        placement: _placement,
        showArrow: widget.showArrow,
        maxWidth: widget.maxWidth ?? 320.0, // max-w-xs
        overlayColor: _overlayColor,
        foreground: _overlayForeground,
        stroke: _arrowStroke,
        shadows: _shadow,
      ),
    );
  }
}

/// Positions the tooltip panel relative to the trigger's rect (global
/// coordinates — the overlay entry fills the screen at the global origin).
class _TooltipPositionDelegate extends SingleChildLayoutDelegate {
  const _TooltipPositionDelegate({
    required this.anchorRect,
    required this.placement,
    required this.gap,
  });

  final Rect anchorRect;
  final HeroTooltipPlacement placement;
  final double gap;

  @override
  BoxConstraints getConstraintsForChild(BoxConstraints constraints) =>
      constraints.loosen();

  @override
  Offset getPositionForChild(Size size, Size childSize) {
    var dx = switch (placement) {
      HeroTooltipPlacement.top || HeroTooltipPlacement.bottom =>
        anchorRect.center.dx - childSize.width / 2,
      HeroTooltipPlacement.left =>
        anchorRect.left - gap - childSize.width,
      HeroTooltipPlacement.right => anchorRect.right + gap,
      _ => 0.0,
    };
    var dy = switch (placement) {
      HeroTooltipPlacement.top => anchorRect.top - gap - childSize.height,
      HeroTooltipPlacement.bottom => anchorRect.bottom + gap,
      HeroTooltipPlacement.left || HeroTooltipPlacement.right =>
        anchorRect.center.dy - childSize.height / 2,
      _ => 0.0,
    };
    // If the requested side has no room, clamp into the visible area so the
    // tooltip is always fully on screen (react-aria keeps the side and
    // shifts it; clamping to a margin is the Flutter analog).
    const margin = 8.0;
    // Guard: clamp(min, max) requires min <= max — if the panel is wider or
    // taller than the overlay, just pin it at the margin.
    final maxX = (size.width - childSize.width - margin) < margin
        ? margin
        : size.width - childSize.width - margin;
    final maxY = (size.height - childSize.height - margin) < margin
        ? margin
        : size.height - childSize.height - margin;
    dx = dx.clamp(margin, maxX);
    dy = dy.clamp(margin, maxY);
    return Offset(dx, dy);
  }

  @override
  bool shouldRelayout(_TooltipPositionDelegate oldDelegate) =>
      oldDelegate.anchorRect != anchorRect ||
      oldDelegate.placement != placement ||
      oldDelegate.gap != gap;
}

/// The tooltip box plus the optional chevron arrow. The arrow's tip sits at
/// the panel edge facing the trigger; the box is padded back so the arrow
/// bridges the gap between the box and the trigger.
class _TooltipPanel extends StatelessWidget {
  const _TooltipPanel({
    required this.message,
    required this.placement,
    required this.showArrow,
    required this.maxWidth,
    required this.overlayColor,
    required this.foreground,
    required this.stroke,
    required this.shadows,
  });

  final String message;
  final HeroTooltipPlacement placement;
  final bool showArrow;
  final double maxWidth;
  final Color overlayColor;
  final Color foreground;
  final Color stroke;
  final List<BoxShadow> shadows;

  static const double _arrowSize = 12.0;
  static const double _protrude = 6.0;

  @override
  Widget build(BuildContext context) {
    final box = Container(
      constraints: BoxConstraints(maxWidth: maxWidth),
      padding: EdgeInsets.all(HeroTokens.space2.resolve(context)), // p-2
      decoration: BoxDecoration(
        color: overlayColor,
        borderRadius: BorderRadius.circular(
          HeroTokens.radiusXl.resolve(context).x, // rounded-xl (12)
        ),
        boxShadow: shadows,
      ),
      // DefaultTextStyle(inherit: false): the overlay entry inherits the
      // host's ambient text style — without this, any decoration (e.g. a
      // link underline) or color from the host would leak into the tooltip.
      child: DefaultTextStyle(
        style: TextStyle(
          fontSize: HeroTokens.doubleTooltipFontSize.resolve(context),
          color: foreground,
          decoration: TextDecoration.none,
        ),
        child: Text(message),
      ),
    );

    if (!showArrow) return box;

    // The arrow lives on the trigger-facing edge; the box is padded back by
    // the arrow protrusion so the arrow tip reaches the panel edge.
    final arrow = Transform.rotate(
      angle: _arrowRotation(placement),
      child: CustomPaint(
        size: const Size.square(_arrowSize),
        painter: _TooltipArrowPainter(fill: overlayColor, stroke: stroke),
      ),
    );

    return Stack(
      clipBehavior: Clip.none,
      children: [
        switch (placement) {
          HeroTooltipPlacement.top => Padding(
              padding: const EdgeInsets.only(bottom: _protrude),
              child: box,
            ),
          HeroTooltipPlacement.bottom => Padding(
              padding: const EdgeInsets.only(top: _protrude),
              child: box,
            ),
          HeroTooltipPlacement.left => Padding(
              padding: const EdgeInsets.only(right: _protrude),
              child: box,
            ),
          HeroTooltipPlacement.right => Padding(
              padding: const EdgeInsets.only(left: _protrude),
              child: box,
            ),
          _ => box,
        },
        switch (placement) {
          HeroTooltipPlacement.top => Positioned(
              bottom: 0,
              left: 0,
              right: 0,
              child: Center(child: arrow),
            ),
          HeroTooltipPlacement.bottom => Positioned(
              top: 0,
              left: 0,
              right: 0,
              child: Center(child: arrow),
            ),
          HeroTooltipPlacement.left => Positioned(
              right: 0,
              top: 0,
              bottom: 0,
              child: Center(child: arrow),
            ),
          HeroTooltipPlacement.right => Positioned(
              left: 0,
              top: 0,
              bottom: 0,
              child: Center(child: arrow),
            ),
          _ => const SizedBox.shrink(),
        },
      ],
    );
  }

  /// Arrow rotation: the chevron's tip points down by default; rotate so the
  /// tip faces the trigger (CSS: bottom→180°, left→-90°, right→90°).
  static double _arrowRotation(HeroTooltipPlacement placement) {
    return switch (placement) {
      HeroTooltipPlacement.top => 0,
      HeroTooltipPlacement.bottom => 3.141592653589793, // 180°
      HeroTooltipPlacement.left => -1.5707963267948966, // -90°
      HeroTooltipPlacement.right => 1.5707963267948966, // 90°
      _ => 0,
    };
  }
}

/// The HeroUI tooltip arrow — `M0 0 C5.48483 8 6.5 8 12 0 Z` (tip at the
/// bottom of the 12×12 canvas), `fill: var(--overlay)`, `stroke: border/40`.
class _TooltipArrowPainter extends CustomPainter {
  const _TooltipArrowPainter({required this.fill, required this.stroke});

  final Color fill;
  final Color stroke;

  @override
  void paint(Canvas canvas, Size size) {
    final path = Path()
      ..moveTo(0, 4)
      ..cubicTo(5.48483, 12, 6.5, 12, 12, 4)
      ..close();
    canvas.drawPath(path, Paint()..style = PaintingStyle.fill..color = fill);
    canvas.drawPath(
      path,
      Paint()
        ..style = PaintingStyle.stroke
        ..strokeWidth = 1
        ..color = stroke,
    );
  }

  @override
  bool shouldRepaint(covariant _TooltipArrowPainter oldDelegate) =>
      oldDelegate.fill != fill || oldDelegate.stroke != stroke;
}
