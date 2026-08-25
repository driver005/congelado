import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';
import 'hero_description.dart';
import 'hero_error_message.dart';

/// A HeroUI v3 checkbox group (checkbox-group.css `.checkbox-group`) — a
/// vertical stack of [HeroCheckbox]es.
///
/// checkbox-group.css: `.checkbox-group` — `flex flex-col`; every
/// `[data-slot="checkbox"]` gets `mt-4` (16), so the group gaps its items.
class HeroCheckboxGroup extends StatelessWidget {
  const HeroCheckboxGroup({
    super.key,
    required this.children,
    this.label,
    this.description,
    this.errorMessage,
  });

  /// The checkboxes (typically [HeroCheckbox] widgets).
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
        for (final child in children)
          Padding(
            padding: EdgeInsets.only(top: HeroTokens.space4.resolve(context)),
            child: child,
          ),
        if (description != null) ...[
          SizedBox(height: HeroTokens.space4.resolve(context)),
          HeroDescription(description!),
        ],
        if (errorMessage != null) ...[
          SizedBox(height: HeroTokens.space4.resolve(context)),
          HeroErrorMessage(errorMessage!),
        ],
      ],
    );
  }
}
