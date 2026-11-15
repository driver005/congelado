import 'package:flutter/material.dart' show showDialog;
import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// Verified against Forui 0.25.0's real source: FDialog dropped its `title`/`body`/`actions`
// params in favor of a single `builder: (context, style) => Widget` — there's no built-in
// title+body+actions layout shipped in the package (only generatable via the `dart run forui
// snippet create adaptive-card` CLI into your own project), so this hand-composes one using
// FDialogStyle's own titleTextStyle/bodyTextStyle. FButton takes `variant:` (FButtonVariant
// enum) directly instead of `style: FButtonStyle.outline`.

/// Shows an imperative confirm/cancel dialog and returns whether the user
/// confirmed — Medusa UI's `usePrompt` hook (`.../ui/src/components/prompt`).
/// Generalizes the delete-confirmation `showDialog`/`FDialog` block that used
/// to be duplicated inline in `task_detail_page.dart` and
/// `workflow_detail_page.dart` — both now call this instead.
///
/// ```dart
/// if (!await showCPrompt(context, title: 'Delete task?', description: 'This removes "$name".')) {
///   return;
/// }
/// ```
Future<bool> showCPrompt(
  BuildContext context, {
  required String title,
  required String description,
  String confirmLabel = 'Confirm',
  String cancelLabel = 'Cancel',
  bool destructive = true,
}) async {
  final confirmed = await showDialog<bool>(
    context: context,
    builder: (dialogContext) => FDialog(
      builder: (context, style) => Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(title, style: style.titleTextStyle),
            const SizedBox(height: 8),
            Text(description, style: style.bodyTextStyle),
            const SizedBox(height: 16),
            Row(
              mainAxisAlignment: MainAxisAlignment.end,
              children: [
                FButton(
                  variant: FButtonVariant.outline,
                  onPress: () => Navigator.pop(dialogContext, false),
                  child: Text(cancelLabel),
                ),
                const SizedBox(width: 8),
                FButton(
                  variant: destructive ? FButtonVariant.destructive : FButtonVariant.primary,
                  onPress: () => Navigator.pop(dialogContext, true),
                  child: Text(confirmLabel),
                ),
              ],
            ),
          ],
        ),
      ),
    ),
  );
  return confirmed == true;
}
