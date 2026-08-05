import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// Verified against Forui 0.25.0's real source (pub.dev archive): FBadge takes a `variant:`
// FBadgeVariant enum directly (no more `style: FBadgeStyle.outline`), and FTheme.of(context)
// .colors is typed FColors (renamed from the FColorScheme this was originally guessed as).

/// Semantic color a [CStatusBadge]'s dot renders in — maps loosely onto
/// Medusa UI's `StatusBadge` `color` prop (`green`/`red`/`orange`/`blue`/
/// `grey`, simplified to five semantic names here since the actual color
/// values come from [FThemeData] rather than a fixed palette).
enum CStatusVariant { success, warning, danger, info, neutral }

/// A colored-dot + label status pill — Medusa UI's `StatusBadge`
/// (`.../ui/src/components/status-badge`). Generalizes the ad-hoc `FBadge`
/// usage already scattered across `tasks_list_page.dart`/
/// `workflow_execution_page.dart` for showing a definition's/instance's
/// status.
class CStatusBadge extends StatelessWidget {
  const CStatusBadge({
    super.key,
    required this.label,
    this.variant = CStatusVariant.neutral,
  });

  final String label;
  final CStatusVariant variant;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return FBadge(
      variant: FBadgeVariant.outline,
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Container(
            width: 6,
            height: 6,
            margin: const EdgeInsets.only(right: 6),
            decoration: BoxDecoration(
              color: cStatusColor(colors, variant),
              shape: BoxShape.circle,
            ),
          ),
          Text(label),
        ],
      ),
    );
  }
}

/// Shared [CStatusVariant] → [Color] mapping — used by [CStatusBadge]'s dot,
/// [CExecutionTimeline]'s bars, and [CWorkflowGraph]'s per-node status stripe,
/// so every status-colored element in the app agrees on the same palette
/// instead of three independent `switch` statements drifting apart.
///
/// Pulled directly from Medusa's own tag-color tokens (`@medusajs/ui-preset`,
/// `theme/tokens/colors.ts`'s `--tag-{green,orange,red,blue,neutral}-text`) —
/// the exact palette Medusa's own `StatusBadge`/`Badge` components use for
/// their five color variants, which [CStatusVariant]'s five names mirror
/// 1:1 (success/warning/danger/info/neutral → green/orange/red/blue/neutral).
/// [FColors] has no equivalent tag-color fields of its own (only the base
/// primary/secondary/muted/destructive/error roles), so these stay literal
/// rather than routed through the theme — same reasoning the previous
/// (unmatched) literals here used, just with real values now. Picks light vs
/// dark off `colors.brightness`, already an [FColors] field.
Color cStatusColor(FColors colors, CStatusVariant variant) {
  final dark = colors.brightness == Brightness.dark;
  switch (variant) {
    case CStatusVariant.success:
      return dark ? const Color(0xFF34D399) : const Color(0xFF065F46); // --tag-green-text
    case CStatusVariant.warning:
      return dark ? const Color(0xFFFDBA74) : const Color(0xFF9A3412); // --tag-orange-text
    case CStatusVariant.danger:
      return dark ? const Color(0xFFFDA4AF) : const Color(0xFF9F1239); // --tag-red-text
    case CStatusVariant.info:
      return dark ? const Color(0xFF93C5FD) : const Color(0xFF1E40AF); // --tag-blue-text
    case CStatusVariant.neutral:
      return dark ? const Color(0xFFD4D4D8) : const Color(0xFF52525B); // --tag-neutral-text
  }
}
