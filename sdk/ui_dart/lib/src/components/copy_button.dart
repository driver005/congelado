import 'package:flutter/services.dart' show Clipboard, ClipboardData;
import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// FButton verified against Forui 0.25.0's real source — takes `variant:` (FButtonVariant enum)
// directly, no more `style: FButtonStyle.ghost`; icons come from FLucideIcons, not FIcons.
// showFToast is still a guess at Forui's imperative toast-trigger API (mirrors Flutter's own
// ScaffoldMessenger.showSnackBar shape) — pairs with CToasterScope (toaster_scope.dart), which
// mounts whatever host widget that API actually needs; reconcile both together if the real API
// differs.
/// Copies [text] to the clipboard on press and shows a brief toast — Medusa
/// UI's `CopyButton` (`.../ui/src/components/copy`).
class CCopyButton extends StatelessWidget {
  const CCopyButton({super.key, required this.text, this.tooltip = 'Copy'});

  final String text;
  final String tooltip;

  Future<void> _copy(BuildContext context) async {
    await Clipboard.setData(ClipboardData(text: text));
    if (!context.mounted) return;
    showFToast(context: context, title: const Text('Copied to clipboard'));
  }

  @override
  Widget build(BuildContext context) {
    return FButton.icon(
      variant: FButtonVariant.ghost,
      onPress: () => _copy(context),
      child: const Icon(FLucideIcons.copy),
    );
  }
}
