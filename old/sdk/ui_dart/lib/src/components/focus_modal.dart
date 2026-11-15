import 'package:flutter/material.dart' show MaterialPageRoute;
import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// Verified against Forui 0.25.0's real source: FTheme.of(context).colors is typed FColors, and
// FButton takes `variant:` (FButtonVariant enum) directly with icons from FLucideIcons. Uses
// Flutter's own well-known `fullscreenDialog: true` route flag rather than a guessed Forui
// full-screen-dialog widget, since that's the standard, high-confidence mechanism for "this push
// should present as a full-screen modal takeover."

/// Pushes [child] as a full-screen modal — Medusa UI's `FocusModal`
/// (`.../ui/src/components/focus-modal`), used for a "step out of the
/// current page entirely" flow (as opposed to [showCPrompt]'s small
/// in-place confirm dialog). [child] is responsible for its own scaffold
/// content; this only supplies the themed close-button header.
Future<T?> showCFocusModal<T>(
  BuildContext context, {
  required String title,
  required WidgetBuilder builder,
}) {
  return Navigator.of(context).push<T>(
    MaterialPageRoute(
      fullscreenDialog: true,
      builder: (context) => _CFocusModalScaffold(title: title, builder: builder),
    ),
  );
}

class _CFocusModalScaffold extends StatelessWidget {
  const _CFocusModalScaffold({required this.title, required this.builder});

  final String title;
  final WidgetBuilder builder;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return ColoredBox(
      color: colors.background,
      child: SafeArea(
        child: Column(
          children: [
            Row(
              children: [
                FButton.icon(
                  variant: FButtonVariant.ghost,
                  onPress: () => Navigator.of(context).pop(),
                  child: const Icon(FLucideIcons.x),
                ),
                const SizedBox(width: 8),
                Text(
                  title,
                  style: TextStyle(fontSize: 16, fontWeight: FontWeight.w600, color: colors.foreground),
                ),
              ],
            ),
            Expanded(child: Builder(builder: builder)),
          ],
        ),
      ),
    );
  }
}
