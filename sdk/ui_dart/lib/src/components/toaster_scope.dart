import 'package:flutter/widgets.dart';

/// Wrap the app root with this — Medusa UI splits `Toast` (the notification
/// content) from `Toaster` (the host/provider that renders queued toasts,
/// same split as react-hot-toast/sonner).
///
/// NOTE: see app/lib/shell/shell_scaffold.dart's top-of-file note. Forui's
/// documented component list only mentions "Toast", not a separate host
/// widget, which likely means `showFToast` (see copy_button.dart) self-mounts
/// via the ambient Navigator Overlay, the same way Flutter's own
/// `showDialog`/`showMenu` need no separate host either. This wrapper is a
/// deliberate placeholder rather than a confirmed no-op: if Forui's real API
/// *does* need an explicit host mounted (check its actual docs/source once
/// `flutter pub get` can run), wrap `child` with that host here — every call
/// site already goes through this one hook point (this wraps the app root in
/// `app/lib/main.dart`) and won't need touching again.
class CToasterScope extends StatelessWidget {
  const CToasterScope({super.key, required this.child});

  final Widget child;

  @override
  Widget build(BuildContext context) => child;
}
