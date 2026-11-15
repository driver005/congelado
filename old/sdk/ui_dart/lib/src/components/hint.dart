import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// NOTE: see app/lib/shell/shell_scaffold.dart's top-of-file note — FTheme.of(context).colors
// field names below are written from Forui's documented component list, not verified against
// the live package.

/// Small muted helper/description text, typically placed under a form field
/// — Medusa UI's `Hint` (`.../ui/src/components/hint`).
class CHint extends StatelessWidget {
  const CHint(this.text, {super.key});

  final String text;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return Text(text, style: TextStyle(fontSize: 12, color: colors.mutedForeground));
  }
}
