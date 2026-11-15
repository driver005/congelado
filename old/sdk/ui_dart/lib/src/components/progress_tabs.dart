import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// Verified against Forui 0.25.0's real source: FTheme.of(context).colors is typed FColors
// (renamed from the FColorScheme this was originally guessed as), and icons come from
// FLucideIcons (FIcons is a per-theme instance, not a static icon namespace). Built from plain
// Row/GestureDetector rather than Forui's FTabs directly — per-tab progress-state decoration
// isn't something FTabs' own API supports, so this composes its own tab strip instead.

/// A step's state in a [CProgressTabs] strip.
enum CProgressTabState { pending, current, completed }

/// A single step in a [CProgressTabs] strip.
class CProgressTab {
  const CProgressTab({required this.label, required this.state});

  final String label;
  final CProgressTabState state;
}

/// A horizontal step/wizard indicator — Medusa UI's `ProgressTabs`
/// (`.../ui/src/components/progress-tabs`). Each step shows a checkmark
/// (completed), a filled dot (current), or an outlined dot (pending).
class CProgressTabs extends StatelessWidget {
  const CProgressTabs({super.key, required this.tabs, this.onTap});

  final List<CProgressTab> tabs;
  final ValueChanged<int>? onTap;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return Row(
      children: [
        for (var index = 0; index < tabs.length; index++) ...[
          if (index > 0)
            Expanded(child: Container(height: 1, color: colors.border)),
          GestureDetector(
            onTap: onTap == null ? null : () => onTap!(index),
            child: _buildStep(colors, tabs[index]),
          ),
        ],
      ],
    );
  }

  Widget _buildStep(FColors colors, CProgressTab tab) {
    late final Widget indicator;
    switch (tab.state) {
      case CProgressTabState.completed:
        indicator = Icon(FLucideIcons.check, size: 14, color: colors.primaryForeground);
      case CProgressTabState.current:
        indicator = const SizedBox.shrink();
      case CProgressTabState.pending:
        indicator = const SizedBox.shrink();
    }
    final bool filled = tab.state != CProgressTabState.pending;
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Container(
          width: 20,
          height: 20,
          alignment: Alignment.center,
          decoration: BoxDecoration(
            shape: BoxShape.circle,
            color: filled ? colors.primary : null,
            border: filled ? null : Border.all(color: colors.border),
          ),
          child: indicator,
        ),
        const SizedBox(width: 8),
        Text(
          tab.label,
          style: TextStyle(
            fontSize: 13,
            fontWeight: tab.state == CProgressTabState.current ? FontWeight.w600 : FontWeight.normal,
            color: tab.state == CProgressTabState.pending ? colors.mutedForeground : colors.foreground,
          ),
        ),
        const SizedBox(width: 16),
      ],
    );
  }
}
