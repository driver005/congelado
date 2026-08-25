import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// A HeroUI v3 error message (error-message.css `.error-message`) — a static
/// error caption used outside of fields.
///
/// error-message.css: `.error-message` — `text-xs text-danger break-words`.
class HeroErrorMessage extends StatelessWidget {
  const HeroErrorMessage(this.message, {super.key, this.maxLines});

  /// The error text.
  final String message;

  /// Optional maximum number of lines before ellipsis.
  final int? maxLines;

  @override
  Widget build(BuildContext context) {
    return Text(
      message,
      maxLines: maxLines,
      overflow: maxLines == null ? null : TextOverflow.ellipsis,
      style: HeroTokens.typeXs.resolve(context).copyWith(
            color: HeroTokens.colorDanger.resolve(context),
          ),
    );
  }
}
