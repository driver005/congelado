import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// A HeroUI v3 field description (description.css `.description`).
///
/// description.css: `.description` — `text-xs text-muted break-words`.
class HeroDescription extends StatelessWidget {
  const HeroDescription(this.text, {super.key, this.maxLines});

  /// The description text.
  final String text;

  /// Optional maximum number of lines before ellipsis.
  final int? maxLines;

  @override
  Widget build(BuildContext context) {
    return Text(
      text,
      maxLines: maxLines,
      overflow: maxLines == null ? null : TextOverflow.ellipsis,
      style: HeroTokens.typeXs.resolve(context).copyWith(
            color: HeroTokens.colorMuted.resolve(context),
          ),
    );
  }
}
