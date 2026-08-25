import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// A HeroUI v3 keyboard key (kbd.css `.kbd`).
///
/// kbd.css: `.kbd` — `inline-flex h-6 items-center whitespace-nowrap rounded-lg
/// bg-default px-2 text-sm font-medium text-muted`; `--light` uses a
/// transparent background.
class HeroKbd extends StatelessWidget {
  const HeroKbd(this.keys, {super.key, this.light = false});

  /// The key(s) to render, e.g. `'⌘ K'`.
  final String keys;

  /// Transparent background (`.kbd--light`).
  final bool light;

  @override
  Widget build(BuildContext context) {
    final style = HeroTokens.typeSm.resolve(context).copyWith(
          fontWeight: HeroTokens.weightMedium.resolve(context),
          color: HeroTokens.colorMuted.resolve(context),
        );
    return Container(
      height: HeroTokens.space6.resolve(context),
      padding: EdgeInsets.symmetric(horizontal: HeroTokens.space2.resolve(context)),
      decoration: BoxDecoration(
        color: light
            ? HeroTokens.colorTransparent.resolve(context)
            : HeroTokens.colorDefault.resolve(context),
        borderRadius: BorderRadius.circular(
          HeroTokens.radiusLg.resolve(context).x,
        ),
      ),
      alignment: Alignment.center,
      child: Text(
        keys,
        style: style.copyWith(fontFamily: 'Inter', letterSpacing: -1),
      ),
    );
  }
}
