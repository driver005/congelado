import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// NOTE: see app/lib/shell/shell_scaffold.dart's top-of-file note — FTheme.of(context).colors
// field names below are written from Forui's documented component list, not verified against
// the live package.

/// A themed surface — background, border, and rounded corners from the
/// shared theme — Medusa UI's `Container` (`.../ui/src/components/container`).
/// Plainer than [FCard]: no built-in padding/elevation opinion, just the
/// surface itself, for callers that want to compose their own layout inside.
class CContainer extends StatelessWidget {
  const CContainer({
    super.key,
    required this.child,
    this.padding = const EdgeInsets.all(16),
  });

  final Widget child;
  final EdgeInsetsGeometry padding;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return Container(
      padding: padding,
      decoration: BoxDecoration(
        color: colors.background,
        border: Border.all(color: colors.border),
        borderRadius: BorderRadius.circular(8),
      ),
      child: child,
    );
  }
}
