import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// A HeroUI v3 field label (label.css `.label`).
///
/// label.css: `.label` — `text-sm font-medium text-foreground`; `--required`
/// appends a danger `*`; `--invalid` recolors the label to danger;
/// `--disabled` fades it.
class HeroLabel extends StatelessWidget {
  const HeroLabel({
    super.key,
    required this.text,
    this.required = false,
    this.invalid = false,
    this.disabled = false,
  });

  /// The label text.
  final String text;

  /// Renders a danger `*` after the text (`.label--required`).
  final bool required;

  /// Colors the label danger (`.label--invalid`).
  final bool invalid;

  /// Fades the label (`.label--disabled`).
  final bool disabled;

  @override
  Widget build(BuildContext context) {
    final style = HeroTokens.typeSm.resolve(context).copyWith(
          fontWeight: HeroTokens.weightMedium.resolve(context),
          color: invalid
              ? HeroTokens.colorDanger.resolve(context)
              : HeroTokens.colorForeground.resolve(context),
        );
    final opacity =
        disabled ? HeroTokens.doubleDisabledOpacity.resolve(context) : 1.0;
    return Opacity(
      opacity: opacity,
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Flexible(child: Text(text, style: style)),
          if (required)
            Padding(
              padding: EdgeInsets.only(left: HeroTokens.space05.resolve(context)),
              child: Text(
                '*',
                style: style.copyWith(color: HeroTokens.colorDanger.resolve(context)),
              ),
            ),
        ],
      ),
    );
  }
}
