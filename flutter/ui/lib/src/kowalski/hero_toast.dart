import 'dart:async';

import 'package:flutter/material.dart';
import 'package:mix/mix.dart';

import '../foundation/hero_color_roles.dart';
import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';
import 'hero_spinner.dart';

/// HeroUI v3 toast colors (toast.css `.toast--<color>`).
///
/// The toast keeps its surface background; only the title (and, for
/// success/warning/danger, the indicator) are tinted with the role's soft
/// foreground. The indicator of `default`/`accent` stays at
/// `--overlay-foreground` (toast.css has no `.toast--accent
/// .toast__indicator` rule).
enum HeroToastColor { accent, default_, success, warning, danger }

/// HeroUI v3 toast placements (toast.css `.toast-region--<placement>`).
enum HeroToastPlacement {
  top,
  topStart,
  topEnd,
  bottom,
  bottomStart,
  bottomEnd,
}

/// Shows a HeroUI v3 toast (toast.css `.toast`) in the root overlay.
///
/// The toast is non-modal — the page behind stays interactive — and is
/// positioned like `.toast-region--<placement>` (16 from the matching
/// edges). It slides/fades in (350ms `--ease-out`, the view-transition
/// timing), auto-dismisses after [duration] (HeroUI's default 4000ms), and
/// completes when dismissed (auto, close button, or [onClose]).
///
/// Requires a `HeroScope` ancestor; tokens are captured from the caller so
/// the overlay entry resolves them correctly.
Future<void> showHeroToast(
  BuildContext context, {
  String? title,
  String? description,
  IconData? icon,
  bool spinner = false,
  HeroToastColor color = HeroToastColor.default_,
  HeroToastPlacement placement = HeroToastPlacement.bottom,
  Duration duration = const Duration(milliseconds: 4000),
  Widget? action,
  VoidCallback? onClose,
}) {
  final scope = MixScope.of(context);
  final overlay = Overlay.of(context, rootOverlay: true);
  final completer = Completer<void>();

  late final OverlayEntry entry;
  // `mounted` only flips false after the entry unmounts (next frame), so a
  // same-frame double dismiss (close button + auto-dismiss) would otherwise
  // trip OverlayEntry's "removed only once" assertion.
  var _removed = false;

  void dismiss() {
    if (!_removed && entry.mounted) {
      _removed = true;
      entry.remove();
    }
    if (!completer.isCompleted) {
      completer.complete();
    }
  }

  entry = OverlayEntry(
    builder: (context) => MixScope(
      tokens: scope.tokens,
      orderOfModifiers: scope.orderOfModifiers,
      child: _HeroToastOverlay(
        title: title,
        description: description,
        icon: icon,
        spinner: spinner,
        color: color,
        placement: placement,
        duration: duration,
        action: action,
        onClose: onClose == null
            ? null
            : () {
                onClose();
                dismiss();
              },
        onDismissed: dismiss,
      ),
    ),
  );
  overlay.insert(entry);

  // Guard: if the overlay is torn down before the toast dismisses (e.g. the
  // navigator's overlay is replaced), still complete the future.
  return completer.future;
}

/// HeroUI v3 toast placement → Flutter [Alignment].
Alignment _heroToastAlignment(HeroToastPlacement placement) {
  return switch (placement) {
    HeroToastPlacement.top => Alignment.topCenter,
    HeroToastPlacement.topStart => Alignment.topLeft,
    HeroToastPlacement.topEnd => Alignment.topRight,
    HeroToastPlacement.bottom => Alignment.bottomCenter,
    HeroToastPlacement.bottomStart => Alignment.bottomLeft,
    HeroToastPlacement.bottomEnd => Alignment.bottomRight,
  };
}

/// Slide-in offset (fraction of the toast size) matching the CSS view
/// transitions: `toast-slide-top-in` (translate 0 -100%) for top placements,
/// `toast-slide-bottom-in` for bottom.
Offset _heroToastSlideOffset(HeroToastPlacement placement) {
  return switch (placement) {
    HeroToastPlacement.top ||
    HeroToastPlacement.topStart ||
    HeroToastPlacement.topEnd => const Offset(0, -1),
    HeroToastPlacement.bottom ||
    HeroToastPlacement.bottomStart ||
    HeroToastPlacement.bottomEnd => const Offset(0, 1),
  };
}

class _HeroToastOverlay extends StatefulWidget {
  const _HeroToastOverlay({
    required this.title,
    required this.description,
    required this.icon,
    required this.spinner,
    required this.color,
    required this.placement,
    required this.duration,
    required this.action,
    required this.onClose,
    required this.onDismissed,
  });

  final String? title;
  final String? description;
  final IconData? icon;
  final bool spinner;
  final HeroToastColor color;
  final HeroToastPlacement placement;
  final Duration duration;
  final Widget? action;
  final VoidCallback? onClose;
  final VoidCallback onDismissed;

  @override
  State<_HeroToastOverlay> createState() => _HeroToastOverlayState();
}

class _HeroToastOverlayState extends State<_HeroToastOverlay> {
  static const _enterMs = 350; // view-transition timing
  static const _toastWidth = 384.0; // --toast-width (HeroUI default)

  bool _visible = false;
  bool _dismissing = false;
  Timer? _autoDismiss;
  Timer? _exitTimer;

  @override
  void initState() {
    super.initState();
    // Enter after the first frame so the slide/fade can animate from 0.
    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (mounted) setState(() => _visible = true);
    });
    if (widget.duration > Duration.zero) {
      _autoDismiss = Timer(widget.duration, _dismiss);
    }
  }

  @override
  void dispose() {
    _autoDismiss?.cancel();
    _exitTimer?.cancel();
    super.dispose();
  }

  void _dismiss() {
    if (_dismissing) return;
    _dismissing = true;
    _autoDismiss?.cancel();
    setState(() => _visible = false);
    final exit = HeroMotion.durationOf(
      context,
      const Duration(milliseconds: _enterMs),
    );
    _exitTimer = Timer(exit, widget.onDismissed);
  }

  @override
  Widget build(BuildContext context) {
    final duration = HeroMotion.durationOf(
      context,
      const Duration(milliseconds: _enterMs),
    );
    final screenWidth = MediaQuery.sizeOf(context).width;
    final isTop =
        widget.placement == HeroToastPlacement.top ||
        widget.placement == HeroToastPlacement.topStart ||
        widget.placement == HeroToastPlacement.topEnd;

    return Align(
      alignment: _heroToastAlignment(widget.placement),
      child: Padding(
        // toast-region offsets: 16 from the matching edges.
        padding: const EdgeInsets.symmetric(horizontal: 16),
        child: SafeArea(
          left: true,
          right: true,
          top: isTop,
          bottom: !isTop,
          child: AnimatedOpacity(
            opacity: _visible ? 1 : 0,
            duration: duration,
            curve: heroEaseOut,
            child: AnimatedSlide(
              offset: _visible
                  ? Offset.zero
                  : _heroToastSlideOffset(widget.placement),
              duration: duration,
              curve: heroEaseOut,
              child: ConstrainedBox(
                constraints: BoxConstraints(
                  // mobile: `w-[calc(100vw-2rem)]`; sm+: `min-w-(--toast-width)`.
                  minWidth: screenWidth < 640 ? screenWidth - 32 : _toastWidth,
                  maxWidth: screenWidth - 32,
                ),
                child: _buildToast(context),
              ),
            ),
          ),
        ),
      ),
    );
  }

  Widget _buildToast(BuildContext context) {
    final titleColor = switch (widget.color) {
      HeroToastColor.default_ => HeroTokens.colorOverlayForeground.resolve(
        context,
      ),
      HeroToastColor.accent => heroColorTokens(
        HeroColor.accent,
      ).softForeground.resolve(context),
      HeroToastColor.success => heroColorTokens(
        HeroColor.success,
      ).softForeground.resolve(context),
      HeroToastColor.warning => heroColorTokens(
        HeroColor.warning,
      ).softForeground.resolve(context),
      HeroToastColor.danger => heroColorTokens(
        HeroColor.danger,
      ).softForeground.resolve(context),
    };
    final indicatorColor = switch (widget.color) {
      // No `.toast--accent .toast__indicator` rule — stays overlay-foreground.
      HeroToastColor.default_ || HeroToastColor.accent =>
        HeroTokens.colorOverlayForeground.resolve(context),
      HeroToastColor.success => heroColorTokens(
        HeroColor.success,
      ).softForeground.resolve(context),
      HeroToastColor.warning => heroColorTokens(
        HeroColor.warning,
      ).softForeground.resolve(context),
      HeroToastColor.danger => heroColorTokens(
        HeroColor.danger,
      ).softForeground.resolve(context),
    };
    final bodySize = HeroTokens.typeSm.resolve(context).fontSize!;

    return Container(
      // toast.css: `.toast` — `bg-surface px-4 py-3 shadow-overlay`, radius
      // `min(32px, var(--radius-3xl))`, `gap-1.5`.
      decoration: BoxDecoration(
        color: HeroTokens.colorSurface.resolve(context),
        borderRadius: BorderRadius.circular(
          HeroTokens.radius3xl.resolve(context).x,
        ),
        boxShadow: HeroTokens.shadowOverlay.resolve(context),
      ),
      padding: EdgeInsets.symmetric(
        horizontal: HeroTokens.space4.resolve(context),
        vertical: HeroTokens.space3.resolve(context),
      ),
      // DefaultTextStyle(decoration: none): overlay content inherits the
      // host's ambient text style — without this, any decoration (e.g. a
      // link underline) leaks into the toast title/description.
      child: DefaultTextStyle(
        style: const TextStyle(decoration: TextDecoration.none),
        child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          if (widget.icon != null || widget.spinner) ...[
            // `.toast__indicator` — `p-1`, icon/spinner `size-4`.
            Padding(
              padding: EdgeInsets.all(HeroTokens.space1.resolve(context)),
              child: widget.spinner
                  ? const HeroSpinner(size: HeroSpinnerSize.sm)
                  : Icon(widget.icon, size: 16, color: indicatorColor),
            ),
            SizedBox(width: HeroTokens.space15.resolve(context)), // gap-1.5
          ],
          Expanded(
            // `.toast__content` — `flex flex-col items-start self-center`.
            child: Column(
              mainAxisSize: MainAxisSize.min,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                if (widget.title != null)
                  Text(
                    widget.title!,
                    // `.toast__title` — `text-sm leading-5 font-medium`.
                    style: TextStyle(
                      fontSize: bodySize,
                      height: 20.0 / bodySize,
                      fontWeight: HeroTokens.weightMedium.resolve(context),
                      color: titleColor,
                    ),
                  ),
                if (widget.description != null)
                  // `.toast__description` — `text-sm text-muted`.
                  Text(
                    widget.description!,
                    style: TextStyle(
                      fontSize: bodySize,
                      color: HeroTokens.colorMuted.resolve(context),
                    ),
                  ),
                if (widget.action != null) ...[
                  // `.toast__action` — `mt-2`.
                  SizedBox(height: HeroTokens.space2.resolve(context)),
                  widget.action!,
                ],
              ],
            ),
          ),
          if (widget.onClose != null) ...[
            SizedBox(width: HeroTokens.space2.resolve(context)),
            _HeroToastCloseButton(onPressed: widget.onClose!),
          ],
        ],
        ),
      ),
    );
  }
}

/// The toast close button (toast.css `.toast__close-button`) — `size-5`
/// (20), `bg-default border-border`, hover `bg-default`, close icon `size-3.5`
/// (14). The CSS reveals it only on hover of the frontmost toast; the Flutter
/// mirror always shows it (see the worksheet).
class _HeroToastCloseButton extends StatefulWidget {
  const _HeroToastCloseButton({required this.onPressed});

  final VoidCallback onPressed;

  @override
  State<_HeroToastCloseButton> createState() => _HeroToastCloseButtonState();
}

class _HeroToastCloseButtonState extends State<_HeroToastCloseButton> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    return HeroFocusRing(
      radius: 10,
      builder: (context, node, focused) => MouseRegion(
        cursor: SystemMouseCursors.click,
        onEnter: (_) => setState(() => _hovered = true),
        onExit: (_) => setState(() => _hovered = false),
        child: Focus(
          focusNode: node,
          child: GestureDetector(
            onTap: widget.onPressed,
            onTapDown: (_) => setState(() => _pressed = true),
            onTapUp: (_) => setState(() => _pressed = false),
            onTapCancel: () => setState(() => _pressed = false),
            child: AnimatedContainer(
              duration: HeroMotion.durationOf(
                context,
                const Duration(milliseconds: 150),
              ),
              curve: heroEaseSmooth,
              width: 20,
              height: 20,
              transform: Matrix4.diagonal3Values(
                _pressed ? 0.93 : 1.0,
                _pressed ? 0.93 : 1.0,
                1.0,
              ),
              decoration: BoxDecoration(
                color: _hovered
                    ? HeroTokens.colorDefaultHover.resolve(context)
                    : HeroTokens.colorDefault.resolve(context),
                borderRadius: BorderRadius.circular(10),
                border: Border.all(
                  color: HeroTokens.colorBorder.resolve(context),
                  width: HeroTokens.doubleBorderWidth.resolve(context),
                ),
              ),
              child: Icon(
                Icons.close_rounded,
                size: 14,
                color: HeroTokens.colorMuted.resolve(context),
              ),
            ),
          ),
        ),
      ),
    );
  }
}
