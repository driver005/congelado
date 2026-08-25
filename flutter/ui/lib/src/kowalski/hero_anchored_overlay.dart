import 'package:flutter/material.dart';
import 'package:mix/mix.dart';

import '../tokens/hero_tokens.dart';

/// A close handle returned by [showHeroAnchoredOverlay].
typedef HeroAnchoredOverlayClose = void Function();

/// Shows [builder]'s content in an overlay anchored to the given [anchorRect]
/// — a popover, NOT a modal: it appears directly below (or above, when there
/// is no room) the anchor, clamped into the visible bounds, and a full-screen
/// transparent barrier dismisses it on outside tap.
///
/// The anchor rect must be measured in the OVERLAY's coordinate space (the
/// nearest Overlay can be a nested one — e.g. the widgetbook
/// DeviceFrameAddon wraps use cases in their own Navigator — whose
/// origin/scale differs from global coordinates).
///
/// The caller supplies the popover chrome (surface, radius, shadow) via
/// [builder]. Returns a close function; the entry is also removed when the
/// barrier is tapped or the overlay is disposed.
HeroAnchoredOverlayClose showHeroAnchoredOverlay(
  BuildContext context, {
  required Rect anchorRect,
  required WidgetBuilder builder,
  double gap = 4,
  VoidCallback? onClosed,
}) {
  final overlay = Overlay.maybeOf(context);
  if (overlay == null) {
    onClosed?.call();
    return () {};
  }
  // Capture the caller's MixScope: the overlay entry renders under the
  // navigator's Overlay, which can sit ABOVE the use case's HeroScope (e.g.
  // the widgetbook DeviceFrameAddon wraps use cases in their own Navigator,
  // and the app's theme addon lives inside the route) — resolving tokens in
  // the entry's own context paints the popover with the wrong theme (e.g. a
  // selected calendar day turning grey).
  final scope = MixScope.of(context);
  late OverlayEntry entry;
  var removed = false;

  void close() {
    if (removed) return;
    removed = true;
    if (entry.mounted) {
      entry.remove();
    }
    onClosed?.call();
  }

  entry = OverlayEntry(
    builder: (entryContext) => MixScope(
      tokens: scope.tokens,
      orderOfModifiers: scope.orderOfModifiers,
      // Build with a context BELOW this MixScope: entryContext is the
      // OverlayEntry's own context (above the MixScope) and would resolve
      // tokens against the outer scope — the "black panel" bug when an outer
      // HeroScope wraps a lighter inner one (widgetbook ThemeAddon).
      child: Builder(
        builder: (panelContext) => Stack(
          children: [
            // Outside-tap barrier (HeroUI popover dismissal).
            Positioned.fill(
              child: GestureDetector(
                behavior: HitTestBehavior.translucent,
                onTap: close,
              ),
            ),
            CustomSingleChildLayout(
              delegate: HeroAnchoredPositionDelegate(
                anchorRect: anchorRect,
                gap: gap,
              ),
              child: builder(panelContext),
            ),
          ],
        ),
      ),
    ),
  );
  overlay.insert(entry);
  return close;
}

/// Positions a popover panel relative to the anchor rect (overlay-local
/// coordinates): below by default, flipping above when there is no room, and
/// clamped into the visible bounds so it is always fully on screen.
class HeroAnchoredPositionDelegate extends SingleChildLayoutDelegate {
  const HeroAnchoredPositionDelegate({
    required this.anchorRect,
    this.gap = 4,
  });

  final Rect anchorRect;
  final double gap;

  @override
  BoxConstraints getConstraintsForChild(BoxConstraints constraints) =>
      constraints.loosen();

  @override
  Offset getPositionForChild(Size size, Size childSize) {
    const margin = 8.0;
    final maxX = (size.width - childSize.width - margin) < margin
        ? margin
        : size.width - childSize.width - margin;
    final maxY = (size.height - childSize.height - margin) < margin
        ? margin
        : size.height - childSize.height - margin;

    final below = anchorRect.bottom + gap + childSize.height <= maxY + margin;
    final dx = (anchorRect.center.dx - childSize.width / 2).clamp(margin, maxX);
    final dy = below
        ? anchorRect.bottom + gap
        : (anchorRect.top - gap - childSize.height).clamp(margin, maxY);
    return Offset(dx, dy);
  }

  @override
  bool shouldRelayout(HeroAnchoredPositionDelegate oldDelegate) =>
      oldDelegate.anchorRect != anchorRect || oldDelegate.gap != gap;
}

/// The popover chrome colors, resolved at the TRIGGER's theme (the navigator
/// Overlay can sit above the use case's HeroScope, so resolving inside the
/// entry would pick the wrong theme).
class HeroPopoverColors {
  const HeroPopoverColors({
    required this.background,
    required this.shadows,
  });

  /// `--overlay` surface.
  final Color background;

  /// `--shadow-overlay` (per-theme).
  final List<BoxShadow> shadows;

  /// Resolves the popover chrome from [context].
  static HeroPopoverColors resolve(BuildContext context) {
    return HeroPopoverColors(
      background: HeroTokens.colorOverlay.resolve(context),
      shadows: HeroTokens.shadowOverlay.resolve(context),
    );
  }
}

/// The HeroUI popover surface (`bg-overlay`, radius 20 — `min(32px,
/// calc(var(--radius) * 2.5))` with `--radius: 8`, per-theme
/// `shadow-overlay`).
class HeroPopoverPanel extends StatelessWidget {
  const HeroPopoverPanel({
    super.key,
    required this.colors,
    required this.child,
    this.padding = 12, // p-3
    this.radius = 20,
  });

  final HeroPopoverColors colors;
  final Widget child;
  final double padding;
  final double radius;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: EdgeInsets.all(padding),
      decoration: BoxDecoration(
        color: colors.background,
        borderRadius: BorderRadius.circular(radius),
        boxShadow: colors.shadows,
      ),
      // DefaultTextStyle(decoration: none): overlay content inherits the
      // host's ambient text style — without this, any decoration (e.g. a
      // link underline) leaks into the popover content.
      child: DefaultTextStyle(
        style: const TextStyle(decoration: TextDecoration.none),
        child: child,
      ),
    );
  }
}
