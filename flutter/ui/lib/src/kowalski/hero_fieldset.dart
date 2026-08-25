import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// A HeroUI v3 fieldset (fieldset.css `.fieldset`) — a vertical group of
/// fields with a consistent gap.
///
/// fieldset.css: `.fieldset` — `flex flex-col grow shrink basis-0 gap-6`.
class HeroFieldset extends StatelessWidget {
  const HeroFieldset({
    super.key,
    required this.children,
    this.gap,
  });

  final List<Widget> children;
  final double? gap;

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: [
        for (var i = 0; i < children.length; i++) ...[
          if (i > 0)
            SizedBox(height: gap ?? HeroTokens.space6.resolve(context)),
          children[i],
        ],
      ],
    );
  }
}
