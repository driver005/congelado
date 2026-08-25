import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';
import 'hero_description.dart';
import 'hero_error_message.dart';

/// A HeroUI v3 tag group (tag-group.css `.tag-group`) — a column that lays
/// out its tag list with an optional label, description and error message.
///
/// tag-group.css: `.tag-group` — `flex flex-col gap-1`; `.tag-group__list` —
/// `flex flex-wrap gap-1.5`; description/error slots get `p-1`.
class HeroTagGroup extends StatelessWidget {
  const HeroTagGroup({
    super.key,
    required this.children,
    this.label,
    this.description,
    this.errorMessage,
  });

  /// The tags (typically [HeroTag] widgets).
  final List<Widget> children;

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
        Wrap(
          spacing: HeroTokens.space15.resolve(context),
          runSpacing: HeroTokens.space15.resolve(context),
          children: children,
        ),
        if (description != null) ...[
          SizedBox(height: gap),
          Padding(
            padding: EdgeInsets.all(HeroTokens.space1.resolve(context)),
            child: HeroDescription(description!),
          ),
        ],
        if (errorMessage != null) ...[
          SizedBox(height: gap),
          Padding(
            padding: EdgeInsets.all(HeroTokens.space1.resolve(context)),
            child: HeroErrorMessage(errorMessage!),
          ),
        ],
      ],
    );
  }
}
