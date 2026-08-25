import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 typography variants (typography.css `.typography--*`).
enum HeroTypographyVariant {
  h1,
  h2,
  h3,
  h4,
  h5,
  h6,
  body,
  bodySm,
  bodyXs,
  code,
}

/// HeroUI v3 typography colors (typography.css `.typography--color-*`).
enum HeroTypographyColor { default_, muted }

/// A HeroUI v3 typography element (typography.css `.typography`) — headings,
/// body text and inline code with the HeroUI type scale.
///
/// typography.css: `.typography` — `text-foreground`; `--h1`…`--h6` —
/// `font-semibold tracking-tight` at 36/30/24/20/18/16; `--body` —
/// `text-base leading-7`; `--body-sm` — `text-sm leading-6`; `--body-xs` —
/// `text-xs leading-5`; `--code` — `rounded-md bg-default px-1.5 py-0.5
/// text-sm font-mono`.
class HeroTypography extends StatelessWidget {
  const HeroTypography(
    this.text, {
    super.key,
    this.variant = HeroTypographyVariant.body,
    this.color = HeroTypographyColor.default_,
    this.align,
    this.maxLines,
    this.truncate = false,
  });

  final String text;
  final HeroTypographyVariant variant;
  final HeroTypographyColor color;
  final TextAlign? align;
  final int? maxLines;

  /// Single-line ellipsis truncation (`.typography--truncate`).
  final bool truncate;

  static bool _isHeading(HeroTypographyVariant v) =>
      v == HeroTypographyVariant.h1 ||
      v == HeroTypographyVariant.h2 ||
      v == HeroTypographyVariant.h3 ||
      v == HeroTypographyVariant.h4 ||
      v == HeroTypographyVariant.h5 ||
      v == HeroTypographyVariant.h6;

  @override
  Widget build(BuildContext context) {
    final (fontSize, lineHeight, fontWeight, mono, inline) = switch (variant) {
      HeroTypographyVariant.h1 => (36.0, 36.0 / 36.0, FontWeight.w600, false, false),
      HeroTypographyVariant.h2 => (30.0, 30.0 / 30.0, FontWeight.w600, false, false),
      HeroTypographyVariant.h3 => (24.0, 24.0 / 24.0, FontWeight.w600, false, false),
      HeroTypographyVariant.h4 => (20.0, 20.0 / 20.0, FontWeight.w600, false, false),
      HeroTypographyVariant.h5 => (18.0, 18.0 / 18.0, FontWeight.w600, false, false),
      HeroTypographyVariant.h6 => (16.0, 16.0 / 16.0, FontWeight.w600, false, false),
      HeroTypographyVariant.body => (16.0, 28.0 / 16.0, null, false, false),
      HeroTypographyVariant.bodySm => (14.0, 24.0 / 14.0, null, false, false),
      HeroTypographyVariant.bodyXs => (12.0, 20.0 / 12.0, null, false, false),
      HeroTypographyVariant.code => (14.0, 1.0, null, true, true),
    };

    final textColor = switch (color) {
      HeroTypographyColor.default_ => HeroTokens.colorForeground.resolve(context),
      HeroTypographyColor.muted => HeroTokens.colorMuted.resolve(context),
    };

    final style = TextStyle(
      fontSize: fontSize,
      height: lineHeight,
      fontWeight: fontWeight,
      fontFamily: mono ? 'monospace' : 'Inter',
      color: textColor,
      // tracking-tight (-0.025em) on headings.
      letterSpacing: _isHeading(variant) ? fontSize * -0.025 : null,
    );

    final content = Text(
      text,
      textAlign: align,
      maxLines: truncate ? 1 : maxLines,
      overflow: truncate || maxLines != null ? TextOverflow.ellipsis : null,
      style: style,
    );

    if (!inline) return content;
    return Container(
      padding: EdgeInsets.symmetric(
        horizontal: HeroTokens.space15.resolve(context),
        vertical: HeroTokens.space05.resolve(context),
      ),
      decoration: BoxDecoration(
        color: HeroTokens.colorDefault.resolve(context),
        borderRadius: BorderRadius.circular(6),
      ),
      child: content,
    );
  }
}
