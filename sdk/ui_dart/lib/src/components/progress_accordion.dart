import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

import 'progress_tabs.dart' show CProgressTabState;

// Verified against Forui 0.25.0's real source: icons come from FLucideIcons (FIcons is a
// per-theme instance, not a static icon namespace).

/// A single section in a [CProgressAccordion].
class CProgressAccordionSection {
  const CProgressAccordionSection({
    required this.title,
    required this.state,
    required this.child,
  });

  final String title;
  final CProgressTabState state;
  final Widget child;
}

/// An accordion whose section headers each show a completion indicator —
/// Medusa UI's `ProgressAccordion` (`.../ui/src/components/progress-accordion`).
/// Reuses [CProgressTabState] rather than a second, near-identical enum.
class CProgressAccordion extends StatelessWidget {
  const CProgressAccordion({super.key, required this.sections});

  final List<CProgressAccordionSection> sections;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return FAccordion(
      children: [
        for (final section in sections)
          FAccordionItem(
            title: Row(
              mainAxisSize: MainAxisSize.min,
              children: [
                Icon(
                  section.state == CProgressTabState.completed ? FLucideIcons.circleCheck : FLucideIcons.circle,
                  size: 16,
                  color: section.state == CProgressTabState.completed
                      ? colors.primary
                      : colors.mutedForeground,
                ),
                const SizedBox(width: 8),
                Text(section.title),
              ],
            ),
            child: section.child,
          ),
      ],
    );
  }
}
