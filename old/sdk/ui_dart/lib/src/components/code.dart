import 'package:flutter/widgets.dart';
import 'package:forui/forui.dart';

import 'copy_button.dart';

// NOTE: see app/lib/shell/shell_scaffold.dart's top-of-file note — FTheme.of(context).colors
// field names below are written from Forui's documented component list, not verified against
// the live package.

/// A short inline monospace code span — Medusa UI's `Code`
/// (`.../ui/src/components/code`), the `<code>`-equivalent for a few words
/// inline with other text (as opposed to [CCodeBlock]'s multi-line block).
class CCode extends StatelessWidget {
  const CCode(this.text, {super.key});

  final String text;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 5, vertical: 1),
      decoration: BoxDecoration(color: colors.muted, borderRadius: BorderRadius.circular(4)),
      child: Text(
        text,
        style: TextStyle(fontFamily: 'monospace', fontSize: 13, color: colors.foreground),
      ),
    );
  }
}

/// A multi-line, monospace code block with an optional copy button — Medusa
/// UI's `CodeBlock` (`.../ui/src/components/code-block`). No syntax
/// highlighting (that's a real, separate feature Medusa's own `CodeBlock`
/// gets from a highlighter library it bundles — out of scope for a first
/// pass here; this is a plain themed monospace block).
class CCodeBlock extends StatelessWidget {
  const CCodeBlock(this.code, {super.key, this.showCopyButton = true});

  final String code;
  final bool showCopyButton;

  @override
  Widget build(BuildContext context) {
    final colors = FTheme.of(context).colors;
    return Container(
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: colors.muted,
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: colors.border),
      ),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Expanded(
            child: SingleChildScrollView(
              scrollDirection: Axis.horizontal,
              child: Text(
                code,
                style: TextStyle(fontFamily: 'monospace', fontSize: 13, color: colors.foreground),
              ),
            ),
          ),
          if (showCopyButton) CCopyButton(text: code, tooltip: 'Copy code'),
        ],
      ),
    );
  }
}
