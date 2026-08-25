import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 modal sizes (modal.css `.modal__dialog--xs/sm/md/lg`).
enum HeroModalSize {
  xs(320),
  sm(384),
  md(448),
  lg(512);

  const HeroModalSize(this.maxWidth);

  /// `max-w-*` in logical px.
  final double maxWidth;
}

final Map<HeroModalSize, RemixDialogStyle> _heroDialogStyleCache = {};

/// Returns the [RemixDialogStyle] for a HeroUI v3 modal dialog.
///
/// modal.css: `.modal__dialog` — `bg-overlay shadow-overlay p-6` (24), radius
/// `min(32px, var(--radius-3xl))` = 24, `max-w-*` per size. The overlay shadow
/// is per-theme; [showHeroModal] resolves it from the scope.
RemixDialogStyle heroDialogStyle({HeroModalSize size = HeroModalSize.md}) {
  return _heroDialogStyleCache.putIfAbsent(size, () {
    return RemixDialogStyle()
        .backgroundColor(HeroTokens.colorOverlay())
        .borderRadius(BorderRadiusGeometryMix.all(HeroTokens.radius3xl()))
        .padding(EdgeInsetsGeometryMix.all(HeroTokens.space6()))
        .constraints(
          BoxConstraintsMix(maxWidth: size.maxWidth),
        );
  });
}

/// Shows a HeroUI v3 modal (modal.css) via the host navigator.
///
/// Requires a `HeroScope` ancestor (like every `Hero*` widget) and a
/// caller-owned `Navigator`. The backdrop uses the theme's `--backdrop`
/// color (rgba(0,0,0,0.5) light / 0.6 dark).
Future<T?> showHeroModal<T>(
  BuildContext context, {
  required WidgetBuilder builder,
  HeroModalSize size = HeroModalSize.md,
  bool barrierDismissible = true,
  String? semanticLabel,
  String? title,
  String? description,
  List<Widget>? actions,
}) {
  final barrierColor = HeroTokens.colorBackdrop.resolve(context);
  return showRemixDialog<T>(
    context: context,
    barrierColor: barrierColor,
    barrierDismissible: barrierDismissible,
    transitionDuration: const Duration(milliseconds: 200),
    builder: (context) {
      var style = heroDialogStyle(size: size);
      final shadows = HeroTokens.shadowOverlay.resolve(context);
      if (shadows.isNotEmpty) {
        style = style.boxShadows([for (final s in shadows) BoxShadowMix.value(s)]);
      }
      // RemixDialog renders ONLY `child` when it is set — title, description
      // and actions would be dropped. Compose them into the child instead so
      // HeroUI's title/description/actions all show.
      return RemixDialog(
        style: style,
        semanticLabel: semanticLabel,
        child: Column(
          mainAxisAlignment: MainAxisAlignment.start,
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            if (title case final title?)
              Padding(
                padding: const EdgeInsets.only(bottom: 8),
                child: Text(
                  title,
                  style: Theme.of(context).textTheme.titleLarge,
                ),
              ),
            if (description case final description?)
              Padding(
                padding: const EdgeInsets.only(bottom: 8),
                child: Text(
                  description,
                  style: Theme.of(context).textTheme.bodyMedium,
                ),
              ),
            builder(context),
            if (actions != null && actions.isNotEmpty)
              Padding(
                padding: const EdgeInsets.only(top: 16),
                child: Row(
                  mainAxisAlignment: MainAxisAlignment.end,
                  children: actions,
                ),
              ),
          ],
        ),
      );
    },
  );
}
