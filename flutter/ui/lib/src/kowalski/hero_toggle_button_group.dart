import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';
import 'hero_toggle_button.dart';

/// A HeroUI v3 toggle button group (toggle-button-group.css
/// `.toggle-button-group`) — a joined (or detached) row/column of
/// [HeroToggleButton]s.
///
/// Attached mode: members are `rounded-none` except the first/last
/// (`rounded-s-3xl` / `rounded-e-3xl`, or top/bottom in vertical), pressed
/// `transform: none`, focus ring inset, and 1px `bg-current opacity-15`
/// separators between members. Detached mode: `gap-1` (4), all members keep
/// their `rounded-3xl` radius and separators are hidden.
class HeroToggleButtonGroup extends StatelessWidget {
  const HeroToggleButtonGroup({
    super.key,
    required this.children,
    this.orientation = Axis.horizontal,
    this.detached = false,
    this.fullWidth = false,
  });

  /// The toggle buttons (typically [HeroToggleButton] widgets).
  final List<Widget> children;

  final Axis orientation;

  /// Detached mode: buttons separated with a gap, each fully rounded.
  final bool detached;

  /// Full width: stretches the group and every member (`flex-1`).
  final bool fullWidth;

  @override
  Widget build(BuildContext context) {
    final separatorColor =
        HeroTokens.colorForeground.resolve(context).withValues(alpha: 0.15);

    Widget separator() => orientation == Axis.horizontal
        ? Container(
            width: 1,
            height: 18, // 50% of the md (36px) button height, centered
            color: separatorColor,
          )
        : Container(
            width: 18,
            height: 1,
            color: separatorColor,
          );

    return Flex(
      direction: orientation,
      mainAxisSize: MainAxisSize.min,
      mainAxisAlignment: MainAxisAlignment.center,
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        for (var i = 0; i < children.length; i++) ...[
          if (i > 0 && !detached) separator(),
          HeroToggleButtonGroupScope(
            orientation: orientation,
            detached: detached,
            index: i,
            count: children.length,
            child: fullWidth ? Expanded(child: children[i]) : children[i],
          ),
        ],
      ],
    );
  }
}
