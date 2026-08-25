import 'package:flutter/material.dart';
import 'package:mix/mix.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';

/// The clear button used inside autocomplete / combo-box triggers
/// (`.autocomplete__clear-button`) — a transparent `size-5` button with a
/// `size-3.5` icon, `bg-default-hover` on hover, fading out when hidden.
class HeroPopupClearButton extends StatefulWidget {
  const HeroPopupClearButton({
    super.key,
    required this.visible,
    required this.onPressed,
    this.iconSize = 14,
  });

  final bool visible;
  final VoidCallback onPressed;
  final double iconSize;

  @override
  State<HeroPopupClearButton> createState() => _HeroPopupClearButtonState();
}

class _HeroPopupClearButtonState extends State<HeroPopupClearButton> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    return AnimatedOpacity(
      duration: HeroMotion.durationOf(
        context,
        const Duration(milliseconds: 150),
      ),
      curve: HeroMotion.smooth,
      opacity: widget.visible ? 1 : 0,
      child: IgnorePointer(
        ignoring: !widget.visible,
        child: MouseRegion(
          cursor: SystemMouseCursors.click,
          onEnter: (_) => setState(() => _hovered = true),
          onExit: (_) => setState(() => _hovered = false),
          child: GestureDetector(
            onTap: widget.visible ? widget.onPressed : null,
            onTapDown: (_) => setState(() => _pressed = true),
            onTapUp: (_) => setState(() => _pressed = false),
            onTapCancel: () => setState(() => _pressed = false),
            child: AnimatedScale(
              scale: _pressed ? 0.93 : 1.0,
              duration: HeroMotion.durationOf(
                context,
                const Duration(milliseconds: 250),
              ),
              curve: heroEaseOutQuart,
              child: AnimatedContainer(
                duration: HeroMotion.durationOf(
                  context,
                  const Duration(milliseconds: 150),
                ),
                curve: HeroMotion.smooth,
                width: 20,
                height: 20,
                decoration: BoxDecoration(
                  color: _hovered
                      ? HeroTokens.colorDefaultHover.resolve(context)
                      : HeroTokens.colorTransparent.resolve(context),
                  borderRadius: BorderRadius.circular(
                    HeroTokens.radiusXl.resolve(context).x,
                  ),
                ),
                child: Icon(
                  Icons.close_rounded,
                  size: widget.iconSize,
                  color: HeroTokens.colorMuted.resolve(context),
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}

/// Controls a [HeroPopup]: lets callers open/close/toggle it programmatically
/// (e.g. a select closes when an option is picked, an autocomplete opens when
/// the field gains focus).
class HeroPopupController extends ChangeNotifier {
  bool _isOpen = false;

  /// Whether the popup is currently shown.
  bool get isOpen => _isOpen;

  /// Opens the popup (no-op when already open).
  void open() {
    if (!_isOpen) {
      _isOpen = true;
      notifyListeners();
    }
  }

  /// Closes the popup (no-op when already closed).
  void close() {
    if (_isOpen) {
      _isOpen = false;
      notifyListeners();
    }
  }

  /// Toggles the popup.
  void toggle() => _isOpen ? close() : open();
}

/// An anchored overlay popup (dropdown).
///
/// Wraps [child] (the trigger) and, when opened, shows [builder]'s content in
/// the nearest [Overlay], aligned below (or above when there is no room) the
/// trigger. Mirrors the HeroUI popover shell used by select / combo-box /
/// autocomplete / menu: `bg-overlay`, radius `min(32px, var(--radius-3xl))`
/// (24), `shadow-overlay`, entering `fade-in-0 zoom-in-95` (150ms,
/// `--ease-smooth`), exiting `fade-out zoom-out-95` (100ms).
class HeroPopup extends StatefulWidget {
  const HeroPopup({
    super.key,
    required this.child,
    required this.builder,
    this.controller,
    this.barrierDismissible = true,
    this.minWidth,
    this.gap = 4,
    this.enterDuration = const Duration(milliseconds: 150),
    this.exitDuration = const Duration(milliseconds: 100),
    this.enterCurve,
    this.exitCurve,
    this.onOpen,
    this.onClose,
  });

  /// The trigger widget (`.popover__trigger`).
  final Widget child;

  /// Builds the popup content. The popup panel chrome (overlay bg, radius,
  /// shadow) is applied here by the caller so each component keeps its own
  /// geometry.
  final WidgetBuilder builder;

  /// Optional controller; when omitted the popup manages its own open state
  /// and toggles on trigger tap.
  final HeroPopupController? controller;

  /// Whether tapping outside the popup closes it.
  final bool barrierDismissible;

  /// Optional minimum popup width — HeroUI popovers use
  /// `min-w-(--trigger-width)`.
  final double? minWidth;

  /// Vertical gap between the trigger and the popup (HeroUI `py-1` = 4).
  final double gap;

  /// Enter/exit animation timings (HeroUI `animate-in duration-150` /
  /// `animate-out duration-100`).
  final Duration enterDuration;
  final Duration exitDuration;

  /// Enter/exit curves. Defaults to `--ease-smooth` for both.
  final Curve? enterCurve;
  final Curve? exitCurve;

  final VoidCallback? onOpen;
  final VoidCallback? onClose;

  @override
  State<HeroPopup> createState() => _HeroPopupState();
}

class _HeroPopupState extends State<HeroPopup> {
  final GlobalKey _targetKey = GlobalKey();
  OverlayEntry? _entry;
  bool _visible = false; // animation state (false while exiting)
  bool _showBelow = true;
  double? _targetWidth;
  Rect _anchorRect = Rect.zero;

  HeroPopupController? _controller;
  HeroPopupController? get controller =>
      widget.controller ?? _controller;

  @override
  void initState() {
    super.initState();
    _controller = widget.controller == null ? HeroPopupController() : null;
    controller!.addListener(_onControllerChanged);
  }

  @override
  void didUpdateWidget(HeroPopup oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.controller != oldWidget.controller) {
      (oldWidget.controller ?? _controller)?.removeListener(_onControllerChanged);
      _controller = widget.controller == null ? HeroPopupController() : null;
      controller!.addListener(_onControllerChanged);
    }
  }

  void _onControllerChanged() {
    if (controller!.isOpen) {
      open();
    } else {
      close();
    }
  }

  @override
  void dispose() {
    controller?.removeListener(_onControllerChanged);
    _controller?.dispose();
    _removeEntry();
    super.dispose();
  }

  void open() {
    if (_entry != null) return;
    final overlay = Overlay.maybeOf(context);
    final targetRenderBox =
        _targetKey.currentContext?.findRenderObject() as RenderBox?;
    final overlayBox = overlay?.context.findRenderObject() as RenderBox?;
    if (overlay == null || targetRenderBox == null || overlayBox == null) {
      return;
    }

    // Measure the trigger in the OVERLAY's coordinate space: the nearest
    // Overlay may be a nested one (e.g. the widgetbook DeviceFrameAddon
    // wraps use cases in their own Navigator), whose origin/scale differs
    // from global coordinates.
    final origin = overlayBox.globalToLocal(
      targetRenderBox.localToGlobal(Offset.zero),
    );
    final targetRect = origin & targetRenderBox.size;
    final overlaySize = overlay.context.size;
    final overlayHeight = overlaySize?.height ?? double.infinity;
    final belowSpace = overlayHeight - targetRect.bottom;
    final aboveSpace = targetRect.top;
    _showBelow = belowSpace >= aboveSpace;
    _targetWidth = targetRect.width;
    _anchorRect = targetRect;

    // Capture the caller's MixScope: the overlay entry renders under the
    // navigator's Overlay, which may sit ABOVE the use case's HeroScope
    // (e.g. a theme addon) — resolving tokens in the entry's own context
    // would paint the popup with the wrong theme.
    final scope = MixScope.of(context);
    _entry = OverlayEntry(
      builder: (entryContext) => MixScope(
        tokens: scope.tokens,
        orderOfModifiers: scope.orderOfModifiers,
        // Build the panel with a context BELOW this MixScope: entryContext is
        // the OverlayEntry's own context (above the MixScope) and would
        // resolve tokens against the outer scope — the "black panel" bug when
        // an outer HeroScope (platform brightness) wraps a lighter inner one
        // (widgetbook ThemeAddon).
        child: Builder(
          builder: (panelContext) => _buildOverlay(panelContext),
        ),
      ),
    );
    overlay.insert(_entry!);
    _visible = true;
    // Force the entry to rebuild once mounted so the entrance animation
    // starts from the hidden state.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (mounted && _entry != null) {
        _entry!.markNeedsBuild();
      }
    });
    widget.onOpen?.call();
  }

  void close() {
    if (_entry == null) return;
    _visible = false;
    // The overlay entry is a separate element tree — a state setState does
    // NOT rebuild it. markNeedsBuild drives the exit transition, whose
    // completion removes the entry.
    _entry!.markNeedsBuild();
    widget.onClose?.call();
  }

  void _removeEntry() {
    final entry = _entry;
    _entry = null;
    // `mounted` is false once the entry's widget state has unmounted — a
    // same-frame double remove would otherwise trip OverlayEntry's
    // "removed only once" assertion.
    if (entry != null && entry.mounted) {
      entry.remove();
    }
  }

  /// Wraps the popup content in a width constraint. Popovers must be as wide
  /// as the trigger (HeroUI `min-w-(--trigger-width)`): without a bound, a
  /// content Column with `crossAxisAlignment: stretch` (e.g. HeroListBox)
  /// would expand to the full overlay width — a giant dark panel. Both min
  /// and max are pinned to the trigger width so the popup can never outgrow
  /// the input.
  Widget _wrapWidth(Widget child) {
    final width = widget.minWidth ?? _targetWidth;
    if (width == null) return child;
    return ConstrainedBox(
      constraints: BoxConstraints(minWidth: width, maxWidth: width),
      child: child,
    );
  }

  Widget _buildOverlay(BuildContext context) {
    // The Overlay lays entries out with BoxConstraints.tight(screen size);
    // the CustomSingleChildLayout gives the panel loose constraints (so it
    // sizes to its content) and positions it from the measured anchor rect
    // (global coordinates — the entry fills the screen at the origin).
    return Stack(
      children: [
        if (widget.barrierDismissible)
          Positioned.fill(
            child: GestureDetector(
              behavior: HitTestBehavior.translucent,
              onTap: close,
            ),
          ),
        CustomSingleChildLayout(
          delegate: _HeroPopupPositionDelegate(
            anchorRect: _anchorRect,
            below: _showBelow,
            gap: widget.gap,
          ),
          child: DefaultTextStyle(
            // Overlay content inherits the host's ambient text style —
            // reset any decoration (e.g. a link underline) so it can't leak.
            style: const TextStyle(decoration: TextDecoration.none),
            child: _PopupTransition(
            visible: _visible,
            onDismissed: _removeEntry,
            enterDuration: widget.enterDuration,
            exitDuration: widget.exitDuration,
            enterCurve: widget.enterCurve ?? HeroMotion.smooth,
            exitCurve: widget.exitCurve ?? HeroMotion.smooth,
            child: _wrapWidth(widget.builder(context)),
          ),
          ),
        ),
      ],
    );
  }

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      key: _targetKey,
      behavior: HitTestBehavior.opaque,
      onTap: controller!.isOpen ? close : open,
      child: widget.child,
    );
  }
}

/// Positions the popup panel relative to the trigger's rect (global
/// coordinates — the overlay entry fills the screen at the global origin).
/// The panel is centered horizontally on the trigger, below it (or above when
/// there is no room below).
class _HeroPopupPositionDelegate extends SingleChildLayoutDelegate {
  const _HeroPopupPositionDelegate({
    required this.anchorRect,
    required this.below,
    required this.gap,
  });

  final Rect anchorRect;
  final bool below;
  final double gap;

  @override
  BoxConstraints getConstraintsForChild(BoxConstraints constraints) =>
      constraints.loosen();

  @override
  Offset getPositionForChild(Size size, Size childSize) {
    var dx = anchorRect.center.dx - childSize.width / 2;
    var dy = below
        ? anchorRect.bottom + gap
        : anchorRect.top - gap - childSize.height;
    // Clamp into the visible area so the popup is always fully on screen.
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
  bool shouldRelayout(_HeroPopupPositionDelegate oldDelegate) =>
      oldDelegate.anchorRect != anchorRect ||
      oldDelegate.below != below ||
      oldDelegate.gap != gap;
}

/// Animates the popup in (fade + zoom 95% → 100%) and out, then reports
/// completion via [onDismissed] so the overlay entry can be removed.
class _PopupTransition extends StatefulWidget {
  const _PopupTransition({
    required this.visible,
    required this.onDismissed,
    required this.enterDuration,
    required this.exitDuration,
    required this.enterCurve,
    required this.exitCurve,
    required this.child,
  });

  final bool visible;
  final VoidCallback onDismissed;
  final Duration enterDuration;
  final Duration exitDuration;
  final Curve enterCurve;
  final Curve exitCurve;
  final Widget child;

  @override
  State<_PopupTransition> createState() => _PopupTransitionState();
}

class _PopupTransitionState extends State<_PopupTransition>
    with SingleTickerProviderStateMixin {
  late final AnimationController _controller;
  late final Animation<double> _opacity;
  late final Animation<double> _scale;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      vsync: this,
      duration: widget.enterDuration,
      reverseDuration: widget.exitDuration,
    );
    _controller.addStatusListener((status) {
      if (status == AnimationStatus.dismissed && !widget.visible) {
        widget.onDismissed();
      }
    });
    _opacity = CurvedAnimation(parent: _controller, curve: widget.enterCurve, reverseCurve: widget.exitCurve);
    _scale = Tween<double>(begin: 0.95, end: 1.0).animate(
      CurvedAnimation(parent: _controller, curve: widget.enterCurve, reverseCurve: widget.exitCurve),
    );
    if (widget.visible) {
      _controller.forward();
    }
  }

  @override
  void didUpdateWidget(_PopupTransition oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.visible && !oldWidget.visible) {
      _controller.forward();
    } else if (!widget.visible && oldWidget.visible) {
      _controller.reverse();
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedBuilder(
      animation: _controller,
      builder: (context, child) => Transform.scale(
        scale: _scale.value,
        child: Opacity(opacity: _opacity.value, child: child),
      ),
      child: widget.child,
    );
  }
}
