import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

// NOTE: see app/lib/shell/shell_scaffold.dart's top-of-file note — FTheme.of(context).colors
// field names below are written from Forui's documented component list, not verified against
// the live package.

/// Heading size scale — Medusa UI's `Heading` `level` prop (`"h1"`..`"h3"`),
/// widened slightly to a 5-step scale since Flutter has no native `<h1>`-`<h6>`
/// concept to mirror 1:1.
enum CHeadingLevel { h1, h2, h3, h4, h5 }

const Map<CHeadingLevel, double> _headingSizes = {
  CHeadingLevel.h1: 30,
  CHeadingLevel.h2: 24,
  CHeadingLevel.h3: 20,
  CHeadingLevel.h4: 17,
  CHeadingLevel.h5: 14,
};

/// A themed heading — Medusa UI's `Heading`
/// (`.../ui/src/components/heading`). Always uses `colors.foreground` and a
/// semi-bold weight; only the size scales with [level].
class CHeading extends StatelessWidget {
  const CHeading(this.text, {super.key, this.level = CHeadingLevel.h2});

  final String text;
  final CHeadingLevel level;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return Text(
      text,
      style: TextStyle(
        fontSize: _headingSizes[level],
        fontWeight: FontWeight.w600,
        color: colors.foreground,
      ),
    );
  }
}

/// Text size/weight variant — Medusa UI's `Text` `size`/`weight` props,
/// collapsed into one enum for the common cases this codebase actually uses.
enum CTextVariant { body, bodyMuted, small, smallMuted }

/// Themed body text — Medusa UI's `Text` (`.../ui/src/components/text`).
class CText extends StatelessWidget {
  const CText(this.text, {super.key, this.variant = CTextVariant.body});

  final String text;
  final CTextVariant variant;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    late final double size;
    late final Color color;
    switch (variant) {
      case CTextVariant.body:
        size = 14;
        color = colors.foreground;
      case CTextVariant.bodyMuted:
        size = 14;
        color = colors.mutedForeground;
      case CTextVariant.small:
        size = 12;
        color = colors.foreground;
      case CTextVariant.smallMuted:
        size = 12;
        color = colors.mutedForeground;
    }
    return Text(text, style: TextStyle(fontSize: size, color: color));
  }
}
