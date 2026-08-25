import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// A HeroUI v3 toolbar (toolbar.css `.toolbar`) — a row of action controls.
///
/// toolbar.css: `.toolbar` — `grid w-fit grid-flow-col items-center gap-2`;
/// `--attached` wraps in a `bg-surface` pill (`rounded-3xl`, `p-1`,
/// `shadow-overlay`); `--vertical` stacks into a column.
class HeroToolbar extends StatelessWidget {
  const HeroToolbar({
    super.key,
    required this.children,
    this.attached = false,
    this.vertical = false,
  });

  final List<Widget> children;
  final bool attached;
  final bool vertical;

  @override
  Widget build(BuildContext context) {
    final gap = HeroTokens.space2.resolve(context);
    final row = vertical
        ? Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: _spaced(children, gap, vertical: true),
          )
        : Row(
            mainAxisSize: MainAxisSize.min,
            children: _spaced(children, gap),
          );
    if (!attached) return row;
    return Container(
      padding: EdgeInsets.all(HeroTokens.space1.resolve(context)),
      decoration: BoxDecoration(
        color: HeroTokens.colorSurface.resolve(context),
        borderRadius: BorderRadius.circular(HeroTokens.radius3xl.resolve(context).x),
        boxShadow: [
          for (final s in HeroTokens.shadowOverlay.resolve(context)) s,
        ],
      ),
      child: row,
    );
  }

  List<Widget> _spaced(List<Widget> items, double gap, {bool vertical = false}) {
    return [
      for (var i = 0; i < items.length; i++) ...[
        if (i > 0)
          SizedBox(
            width: vertical ? 0 : gap,
            height: vertical ? gap : 0,
          ),
        items[i],
      ],
    ];
  }
}
