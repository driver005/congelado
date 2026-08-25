import 'dart:ui' show ImageFilter;

import 'package:flutter/material.dart';
import 'package:mix/mix.dart';

import '../tokens/hero_tokens.dart';
import 'hero_close_button.dart';

/// HeroUI v3 drawer placements (drawer.css `.drawer__content--*`).
enum HeroDrawerPlacement {
  top,
  right,
  bottom,
  left,
}

/// HeroUI v3 drawer backdrop variants (drawer.css `.drawer__backdrop--*`).
enum HeroDrawerBackdrop {
  /// No dimming (`bg-transparent`).
  transparent,

  /// `bg-backdrop` — the theme's dim overlay (default).
  opaque,

  /// `bg-backdrop backdrop-blur-md` — dimmed and blurred.
  blur,
}

/// A HeroUI v3 drawer (drawer.css) — the dialog chrome rendered inside
/// [showHeroDrawer]: a `bg-overlay shadow-overlay p-6` column with an
/// optional heading, body, actions and close button.
///
/// `.drawer__dialog` — `flex flex-col`, `pointer-events-auto`; the close
/// trigger sits at `end-4 top-4`.
class HeroDrawer extends StatelessWidget {
  const HeroDrawer({
    super.key,
    this.title,
    this.description,
    this.actions,
    this.onClose,
    this.placement = HeroDrawerPlacement.right,
    this.showCloseButton = true,
    this.showHandle = false,
    this.semanticLabel,
    this.child,
  });

  /// The drawer heading (`.drawer__heading` — `text-base font-medium
  /// text-foreground`).
  final String? title;

  /// Optional sub-copy shown under the title.
  final String? description;

  /// Footer actions (`.drawer__footer` — `flex-row justify-end gap-2`).
  final List<Widget>? actions;

  /// Invoked by the close button / barrier; callers pop the route here.
  final VoidCallback? onClose;

  final HeroDrawerPlacement placement;

  /// Renders the close button at `end-4 top-4`.
  final bool showCloseButton;

  /// Renders the drag handle bar (`.drawer__handle`).
  final bool showHandle;

  final String? semanticLabel;

  /// The body content (`.drawer__body` — `text-sm text-muted`, scrollable).
  final Widget? child;

  @override
  Widget build(BuildContext context) {
    final header = (title != null || description != null)
        ? Padding(
            // `.drawer__header` — `flex flex-col gap-3`.
            padding: const EdgeInsets.only(bottom: 8),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                if (title != null)
                  Text(
                    title!,
                    style: TextStyle(
                      fontSize: HeroTokens.typeBase.resolve(context).fontSize,
                      fontWeight: HeroTokens.weightMedium.resolve(context),
                      color: HeroTokens.colorForeground.resolve(context),
                    ),
                  ),
                if (description != null)
                  Text(
                    description!,
                    style: TextStyle(
                      fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                      height: 1.43,
                      color: HeroTokens.colorMuted.resolve(context),
                    ),
                  ),
              ],
            ),
          )
        : null;

    final body = child == null
        ? null
        : Expanded(
            // `.drawer__body` — `min-h-0 flex-1 overflow-y-auto text-sm
            // text-muted`.
            child: SingleChildScrollView(
              child: DefaultTextStyle.merge(
                style: TextStyle(
                  fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                  height: 1.43,
                  color: HeroTokens.colorMuted.resolve(context),
                ),
                child: child!,
              ),
            ),
          );

    final footer = (actions == null || actions!.isEmpty)
        ? null
        : Padding(
            // `.drawer__body + .drawer__footer` / `.drawer__header +
            // .drawer__footer` — `mt-5`; `.drawer__footer` —
            // `flex-row items-center justify-end gap-2`.
            padding: const EdgeInsets.only(top: 20),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.end,
              children: [
                for (var i = 0; i < actions!.length; i++) ...[
                  if (i > 0) const SizedBox(width: 8),
                  actions![i],
                ],
              ],
            ),
          );

    final panel = Semantics(
      label: semanticLabel,
      container: true,
      // The Stack fills the `p-6` dialog container; the close trigger is
      // positioned against the dialog edges (`end-4 top-4`), not the
      // content padding.
      child: Stack(
        children: [
          Padding(
            padding: const EdgeInsets.all(24), // p-6
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: [
                if (showHandle) ...[
                  // `.drawer__handle` — centered bar `h-1 w-9 rounded-xs
                  // bg-separator`, `pb-2`.
                  Padding(
                    padding: const EdgeInsets.only(bottom: 8),
                    child: Center(
                      child: Container(
                        width: 36,
                        height: 4,
                        decoration: BoxDecoration(
                          color: HeroTokens.colorSeparator.resolve(context),
                          borderRadius: BorderRadius.circular(
                            HeroTokens.radiusXs.resolve(context).x,
                          ),
                        ),
                      ),
                    ),
                  ),
                ],
                if (header != null) header,
                if (body != null) body,
                if (footer != null) footer,
              ],
            ),
          ),
          if (showCloseButton)
            Positioned(
              top: 16,
              right: 16,
              child: HeroCloseButton(onPressed: onClose),
            ),
        ],
      ),
    );

    return panel;
  }
}

/// Shows a HeroUI v3 drawer (drawer.css) — a panel that slides in from an
/// edge over a dimmed backdrop.
///
/// Requires a caller-owned `Navigator` and a `HeroScope` ancestor (token
/// scope is carried into the route). The drawer animates with the spec's
/// `--drawer-enter-ease` (250ms `cubic-bezier(0.32, 0.72, 0, 1)`).
Future<T?> showHeroDrawer<T>(
  BuildContext context, {
  required WidgetBuilder builder,
  HeroDrawerPlacement placement = HeroDrawerPlacement.right,
  HeroDrawerBackdrop backdrop = HeroDrawerBackdrop.opaque,
  bool barrierDismissible = true,
  bool showHandle = false,
  String? semanticLabel,
  String? title,
  String? description,
  List<Widget>? actions,
}) {
  final scope = MixScope.of(context);
  final backdropColor = HeroTokens.colorBackdrop.resolve(context);

  return showGeneralDialog<T>(
    context: context,
    // The backdrop is painted by the page (blur variant needs a
    // BackdropFilter); the route barrier stays transparent but still
    // absorbs outside taps for barrier dismissal.
    barrierColor: HeroTokens.colorTransparent.resolve(context),
    barrierDismissible: barrierDismissible,
    barrierLabel: semanticLabel ?? 'Dismiss drawer',
    transitionDuration: const Duration(milliseconds: 250),
    // The route's own fade would double-fade the panel; the drawer animates
    // its backdrop + slide itself from the route animation.
    transitionBuilder:
        (context, animation, secondaryAnimation, child) => child,
    pageBuilder: (context, animation, secondaryAnimation) {
      return MixScope(
        tokens: scope.tokens,
        orderOfModifiers: scope.orderOfModifiers,
        child: _HeroDrawerRoute(
          placement: placement,
          backdrop: backdrop,
          backdropColor: backdropColor,
          barrierDismissible: barrierDismissible,
          animation: animation,
          child: HeroDrawer(
            title: title,
            description: description,
            actions: actions,
            placement: placement,
            showHandle: showHandle,
            semanticLabel: semanticLabel,
            onClose: () => Navigator.of(context).pop(),
            child: builder(context),
          ),
        ),
      );
    },
  );
}

class _HeroDrawerRoute extends StatelessWidget {
  const _HeroDrawerRoute({
    required this.placement,
    required this.backdrop,
    required this.backdropColor,
    required this.barrierDismissible,
    required this.animation,
    required this.child,
  });

  final HeroDrawerPlacement placement;
  final HeroDrawerBackdrop backdrop;
  final Color backdropColor;
  final bool barrierDismissible;
  final Animation<double> animation;
  final Widget child;

  @override
  Widget build(BuildContext context) {
    final screen = MediaQuery.sizeOf(context);
    final eased = CurvedAnimation(
      parent: animation,
      curve: heroEaseOutFluid,
      reverseCurve: heroEaseOutFluid,
    );

    // `.drawer__dialog` — `w-80 max-w-[85vw] sm:w-96` for left/right
    // (`w-96` from the 640px breakpoint), `w-full max-h-[85vh]` for
    // top/bottom. Radius: `--radius-2xl` (16) on the anchored edge corners.
    final sideWidth = (screen.width >= 640 ? 384.0 : 320.0)
        .clamp(0.0, screen.width * 0.85)
        .toDouble();
    final bottomRadius = placement == HeroDrawerPlacement.bottom;
    final topRadius = placement == HeroDrawerPlacement.top;

    final panel = Container(
      width: placement == HeroDrawerPlacement.left ||
              placement == HeroDrawerPlacement.right
          ? sideWidth
          : screen.width,
      height: placement == HeroDrawerPlacement.left ||
              placement == HeroDrawerPlacement.right
          ? screen.height
          : null,
      constraints: placement == HeroDrawerPlacement.left ||
              placement == HeroDrawerPlacement.right
          ? const BoxConstraints()
          : BoxConstraints(maxHeight: screen.height * 0.85),
      // The dialog's `p-6` lives inside [HeroDrawer].
      decoration: BoxDecoration(
        color: HeroTokens.colorOverlay.resolve(context),
        borderRadius: BorderRadius.vertical(
          top: Radius.circular(topRadius ? 16 : 0),
          bottom: Radius.circular(bottomRadius ? 16 : 0),
        ),
        boxShadow: [
          for (final s in HeroTokens.shadowOverlay.resolve(context)) s,
        ],
      ),
      child: child,
    );

    final translate = switch (placement) {
      HeroDrawerPlacement.right => Offset(1 - eased.value, 0),
      HeroDrawerPlacement.left => Offset(-(1 - eased.value), 0),
      HeroDrawerPlacement.bottom => Offset(0, 1 - eased.value),
      HeroDrawerPlacement.top => Offset(0, -(1 - eased.value)),
    };

    return Stack(
      children: [
        // `.drawer__backdrop` — fullscreen dim, opacity transition on the
        // spec's enter ease.
        Positioned.fill(
          child: GestureDetector(
            behavior: HitTestBehavior.opaque,
            onTap: barrierDismissible
                ? () => Navigator.of(context).pop()
                : null,
            child: FadeTransition(
              opacity: eased,
              child: backdrop == HeroDrawerBackdrop.blur
                  ? BackdropFilter(
                      filter: ImageFilter.blur(
                        sigmaX: 12,
                        sigmaY: 12,
                      ),
                      child: Container(color: backdropColor),
                    )
                  : Container(
                      color: backdrop == HeroDrawerBackdrop.transparent
                          ? HeroTokens.colorTransparent.resolve(context)
                          : backdropColor,
                    ),
            ),
          ),
        ),
        // `.drawer__content` — fullscreen positioning wrapper for the panel.
        // FractionalTranslation slides the PANEL by its own size
        // (translate: 100%), not the fullscreen wrapper.
        Positioned.fill(
          child: Align(
            alignment: switch (placement) {
              HeroDrawerPlacement.right => Alignment.centerRight,
              HeroDrawerPlacement.left => Alignment.centerLeft,
              HeroDrawerPlacement.bottom => Alignment.bottomCenter,
              HeroDrawerPlacement.top => Alignment.topCenter,
            },
            child: FractionalTranslation(
              translation: translate,
              child: panel,
            ),
          ),
        ),
      ],
    );
  }
}
