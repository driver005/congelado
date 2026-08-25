import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';
import 'hero_description.dart';
import 'hero_error_message.dart';

/// A HeroUI v3 switch group (switch-group.css `.switch-group`) — a column of
/// [HeroSwitch]es (or a row via [orientation]).
///
/// switch-group.css: `.switch-group` — `flex flex-col gap-6` (24);
/// `.switch-group__items` — `flex gap-4` (16), `flex-row`/`flex-col` per
/// orientation.
class HeroSwitchGroup extends StatelessWidget {
  const HeroSwitchGroup({
    super.key,
    required this.children,
    this.orientation = Axis.vertical,
    this.label,
    this.description,
    this.errorMessage,
  });

  /// The switches (typically [HeroSwitch] widgets).
  final List<Widget> children;

  /// `.switch-group--horizontal` / `--vertical`.
  final Axis orientation;

  final Widget? label;
  final String? description;
  final String? errorMessage;

  @override
  Widget build(BuildContext context) {
    final gap = HeroTokens.space1.resolve(context);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        if (label != null) label!,
        if (label != null) SizedBox(height: gap),
        Flex(
          direction: orientation,
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment:
              orientation == Axis.horizontal ? CrossAxisAlignment.center : CrossAxisAlignment.start,
          children: [
            for (var i = 0; i < children.length; i++) ...[
              if (i > 0)
                SizedBox(
                  width: orientation == Axis.horizontal
                      ? HeroTokens.space4.resolve(context)
                      : 0,
                  height: orientation == Axis.vertical
                      ? HeroTokens.space4.resolve(context)
                      : 0,
                ),
              children[i],
            ],
          ],
        ),
        if (description != null) ...[
          SizedBox(height: HeroTokens.space6.resolve(context)),
          HeroDescription(description!),
        ],
        if (errorMessage != null) ...[
          SizedBox(height: HeroTokens.space6.resolve(context)),
          HeroErrorMessage(errorMessage!),
        ],
      ],
    );
  }
}
