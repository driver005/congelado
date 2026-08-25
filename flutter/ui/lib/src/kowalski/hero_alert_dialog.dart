import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../foundation/hero_color_roles.dart';
import '../tokens/hero_tokens.dart';
import 'hero_close_button.dart';

/// HeroUI v3 alert-dialog sizes (alert-dialog.css `.alert-dialog__dialog--*`).
///
/// Same `max-w-*` scale as the modal (xs 320 / sm 384 / md 448 / lg 512).
enum HeroAlertDialogSize {
  xs(320),
  sm(384),
  md(448),
  lg(512);

  const HeroAlertDialogSize(this.maxWidth);

  /// `max-w-*` in logical px.
  final double maxWidth;
}

/// HeroUI v3 alert-dialog backdrop modifiers
/// (alert-dialog.css `.alert-dialog__backdrop--*`).
enum HeroAlertDialogBackdrop {
  /// Dimmed overlay using the theme `--backdrop` color (default).
  opaque,

  /// No backdrop tint.
  transparent,

  /// `--backdrop` plus `backdrop-blur-md`.
  blur,
}

/// HeroUI v3 alert-dialog icon colors (alert-dialog.css
/// `.alert-dialog__icon--<color>`).
enum HeroAlertDialogColor { accent, default_, success, warning, danger }

final Map<HeroAlertDialogSize, RemixDialogStyle> _heroAlertDialogStyleCache =
    {};

/// Returns the [RemixDialogStyle] for a HeroUI v3 alert dialog.
///
/// alert-dialog.css: `.alert-dialog__dialog` — `bg-overlay shadow-overlay
/// p-6` (24), radius `min(32px, var(--radius-3xl))` = 24, `max-w-*` per size.
/// The overlay shadow is per-theme; [showHeroAlertDialog] resolves it from
/// the scope.
RemixDialogStyle heroAlertDialogStyle({
  HeroAlertDialogSize size = HeroAlertDialogSize.md,
}) {
  return _heroAlertDialogStyleCache.putIfAbsent(size, () {
    return RemixDialogStyle()
        .backgroundColor(HeroTokens.colorOverlay())
        .borderRadius(BorderRadiusGeometryMix.all(HeroTokens.radius3xl()))
        .padding(EdgeInsetsGeometryMix.all(HeroTokens.space6()))
        .constraints(BoxConstraintsMix(maxWidth: size.maxWidth));
  });
}

/// A HeroUI v3 alert-dialog (alert-dialog.css) — a modal overlay with an
/// optional icon, heading, description body and a right-aligned action
/// footer. The icon tints its soft chip per [HeroAlertDialogColor].
///
/// Use [showHeroAlertDialog] to present it; this widget is the content
/// surfaced inside the dialog route (mirroring how [HeroModal]'s child is
/// composed).
class HeroAlertDialog extends StatelessWidget {
  const HeroAlertDialog({
    super.key,
    this.title,
    this.description,
    this.icon,
    this.color = HeroAlertDialogColor.default_,
    this.actions = const [],
    this.onClose,
  });

  /// The dialog heading (`.alert-dialog__heading` — `text-base font-medium
  /// text-foreground`).
  final String? title;

  /// The dialog body (`.alert-dialog__body` — `text-sm text-muted`).
  final String? description;

  /// Optional leading icon (`.alert-dialog__icon` — `size-10 rounded-3xl`).
  final IconData? icon;

  /// Icon chip color (`.alert-dialog__icon--<color>`).
  final HeroAlertDialogColor color;

  /// Footer actions (`.alert-dialog__footer` — right-aligned `gap-2`).
  final List<Widget> actions;

  /// When non-null, renders the close trigger (`.alert-dialog__close-trigger`
  /// — `absolute end-4 top-4`).
  final VoidCallback? onClose;

  @override
  Widget build(BuildContext context) {
    final content = Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        // `.alert-dialog__header` — `flex flex-col gap-3`.
        if (icon != null) ...[
          _HeroAlertDialogIcon(icon: icon!, color: color),
          SizedBox(height: HeroTokens.space3.resolve(context)),
        ],
        if (title != null)
          Text(
            title!,
            style: TextStyle(
              fontSize: HeroTokens.typeBase.resolve(context).fontSize,
              fontWeight: HeroTokens.weightMedium.resolve(context),
              color: HeroTokens.colorForeground.resolve(context),
            ),
          ),
        if (description != null) ...[
          // `.alert-dialog__header + .alert-dialog__body` — `mt-2`.
          SizedBox(height: HeroTokens.space2.resolve(context)),
          Text(
            description!,
            // `.alert-dialog__body` — `text-sm leading-[1.43] text-muted`.
            style: TextStyle(
              fontSize: HeroTokens.typeSm.resolve(context).fontSize,
              height: 1.43,
              color: HeroTokens.colorMuted.resolve(context),
            ),
          ),
        ],
        if (actions.isNotEmpty) ...[
          // `.alert-dialog__header/__body + .alert-dialog__footer` — `mt-5`.
          SizedBox(height: HeroTokens.space5.resolve(context)),
          // `.alert-dialog__footer` — `flex-row items-center justify-end
          // gap-2`.
          Row(
            mainAxisAlignment: MainAxisAlignment.end,
            children: [
              for (final (i, action) in actions.indexed) ...[
                if (i > 0) SizedBox(width: HeroTokens.space2.resolve(context)),
                action,
              ],
            ],
          ),
        ],
      ],
    );

    if (onClose == null) {
      return content;
    }

    // `.alert-dialog__close-trigger` — `absolute end-4 top-4`.
    return Stack(
      children: [
        content,
        Positioned(
          top: HeroTokens.space4.resolve(context),
          right: HeroTokens.space4.resolve(context),
          child: HeroCloseButton(onPressed: onClose),
        ),
      ],
    );
  }
}

/// `.alert-dialog__icon` — `size-10 shrink-0 rounded-3xl select-none` with a
/// `size-5` icon; background/foreground per `.alert-dialog__icon--<color>`.
class _HeroAlertDialogIcon extends StatelessWidget {
  const _HeroAlertDialogIcon({required this.icon, required this.color});

  final IconData icon;
  final HeroAlertDialogColor color;

  @override
  Widget build(BuildContext context) {
    final (background, foreground) = switch (color) {
      HeroAlertDialogColor.default_ => (
        HeroTokens.colorDefault.resolve(context),
        HeroTokens.colorForeground.resolve(context),
      ),
      HeroAlertDialogColor.accent => (
        heroColorTokens(HeroColor.accent).soft.resolve(context),
        heroColorTokens(HeroColor.accent).softForeground.resolve(context),
      ),
      HeroAlertDialogColor.success => (
        heroColorTokens(HeroColor.success).soft.resolve(context),
        heroColorTokens(HeroColor.success).softForeground.resolve(context),
      ),
      HeroAlertDialogColor.warning => (
        heroColorTokens(HeroColor.warning).soft.resolve(context),
        heroColorTokens(HeroColor.warning).softForeground.resolve(context),
      ),
      HeroAlertDialogColor.danger => (
        heroColorTokens(HeroColor.danger).soft.resolve(context),
        heroColorTokens(HeroColor.danger).softForeground.resolve(context),
      ),
    };
    final size = HeroTokens.space10.resolve(context); // size-10 = 40
    return Container(
      width: size,
      height: size,
      decoration: BoxDecoration(
        color: background,
        borderRadius: BorderRadius.circular(
          HeroTokens.radius3xl.resolve(context).x,
        ),
      ),
      child: Icon(
        icon,
        size: HeroTokens.space5.resolve(context), // size-5 = 20
        color: foreground,
      ),
    );
  }
}

/// Shows a HeroUI v3 alert-dialog (alert-dialog.css) via the host navigator.
///
/// Requires a `HeroScope` ancestor (like every `Hero*` widget) and a
/// caller-owned `Navigator`. The opaque backdrop uses the theme's `--backdrop`
/// color; `blur` renders the same tint (Flutter's dialog barrier cannot be
/// blurred — see the worksheet) and `transparent` paints none.
Future<T?> showHeroAlertDialog<T>(
  BuildContext context, {
  String? title,
  String? description,
  IconData? icon,
  HeroAlertDialogColor color = HeroAlertDialogColor.default_,
  List<Widget> actions = const [],
  VoidCallback? onClose,
  HeroAlertDialogSize size = HeroAlertDialogSize.md,
  HeroAlertDialogBackdrop backdrop = HeroAlertDialogBackdrop.opaque,
  bool barrierDismissible = true,
  String? semanticLabel,
}) {
  final barrierColor = switch (backdrop) {
    HeroAlertDialogBackdrop.opaque ||
    HeroAlertDialogBackdrop.blur => HeroTokens.colorBackdrop.resolve(context),
    HeroAlertDialogBackdrop.transparent => HeroTokens.colorTransparent.resolve(
      context,
    ),
  };
  return showRemixDialog<T>(
    context: context,
    barrierColor: barrierColor,
    barrierDismissible: barrierDismissible,
    // `.alert-dialog__container` entering animation — `animate-in
    // duration-250 ease-out-quad fade-in-0 zoom-in-105`.
    transitionDuration: const Duration(milliseconds: 250),
    builder: (dialogContext) {
      var style = heroAlertDialogStyle(size: size);
      final shadows = HeroTokens.shadowOverlay.resolve(dialogContext);
      if (shadows.isNotEmpty) {
        style = style.boxShadows([
          for (final s in shadows) BoxShadowMix.value(s),
        ]);
      }
      return RemixDialog(
        style: style,
        semanticLabel: semanticLabel ?? title,
        child: HeroAlertDialog(
          title: title,
          description: description,
          icon: icon,
          color: color,
          actions: actions,
          // The close trigger pops the dialog route with a DIALOG-bound
          // context — never a caller context, which may be stale after the
          // host tree rebuilds. The user callback (if any) runs first.
          onClose: onClose == null
              ? null
              : () {
                  onClose();
                  Navigator.pop(dialogContext);
                },
        ),
      );
    },
  );
}
