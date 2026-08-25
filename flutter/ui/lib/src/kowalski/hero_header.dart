import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// A HeroUI v3 group header (header.css `.header`) — the caption above a
/// section of grouped options (e.g. list-box or menu sections).
///
/// header.css: `.header` — `w-full px-2 pb-1 pt-1.5 text-xs font-medium
/// text-muted`.
class HeroHeader extends StatelessWidget {
  const HeroHeader(this.text, {super.key});

  /// The header text.
  final String text;

  @override
  Widget build(BuildContext context) {
    return Align(
      alignment: Alignment.centerLeft,
      child: Padding(
        padding: EdgeInsets.fromLTRB(
          HeroTokens.space2.resolve(context),
          HeroTokens.space15.resolve(context),
          HeroTokens.space2.resolve(context),
          HeroTokens.space1.resolve(context),
        ),
        child: Text(
          text,
          style: HeroTokens.typeXs.resolve(context).copyWith(
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorMuted.resolve(context),
              ),
        ),
      ),
    );
  }
}
