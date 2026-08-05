import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// NOTE: see app/lib/shell/shell_scaffold.dart's top-of-file note — FTheme.of(context).colors
// field names below are written from Forui's documented component list, not verified against
// the live package.

/// A small rounded chip holding a single icon — Medusa UI's `IconBadge`
/// (`.../ui/src/components/icon-badge`). Used for a compact, badge-styled
/// icon indicator (as opposed to [CIconButton]'s tappable variant).
class CIconBadge extends StatelessWidget {
  const CIconBadge({super.key, required this.icon, this.size = 20});

  final IconData icon;
  final double size;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return Container(
      padding: const EdgeInsets.all(4),
      decoration: BoxDecoration(
        color: colors.secondary,
        borderRadius: BorderRadius.circular(6),
      ),
      child: Icon(icon, size: size, color: colors.secondaryForeground),
    );
  }
}
