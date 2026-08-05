import 'package:flutter/material.dart' show Icons;
import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// NOTE: see app/lib/shell/shell_scaffold.dart's top-of-file note — FTheme.of(context).colors
// field names below are written from Forui's documented component list, not verified against
// the live package.

/// A compact icon + text tip with a subtle background — Medusa UI's
/// `InlineTip` (`.../ui/src/components/inline-tip`). More lightweight than a
/// full `FAlert`; meant for a short one-line note inline with other content
/// rather than a standalone callout box.
class CInlineTip extends StatelessWidget {
  const CInlineTip(this.text, {super.key, this.icon = Icons.info_outline});

  final String text;
  final IconData icon;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 6),
      decoration: BoxDecoration(
        color: colors.muted,
        borderRadius: BorderRadius.circular(6),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Icon(icon, size: 16, color: colors.mutedForeground),
          const SizedBox(width: 8),
          Flexible(
            child: Text(text, style: TextStyle(fontSize: 13, color: colors.mutedForeground)),
          ),
        ],
      ),
    );
  }
}
