import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';

/// A HeroUI v3 empty state (empty-state.css `.empty-state`) — a simple
/// `p-2 text-sm text-muted` container for empty content, with an optional
/// icon above the message.
class HeroEmptyState extends StatelessWidget {
  const HeroEmptyState({
    super.key,
    this.icon,
    required this.message,
    this.alignment = Alignment.center,
  });

  /// Optional icon rendered above the message (muted, same size as the text
  /// line — the CSS does not size it, see the worksheet).
  final IconData? icon;

  /// The empty-state message (`.empty-state` — `text-sm text-muted`).
  final String message;

  /// How the content is aligned within the available space.
  final AlignmentGeometry alignment;

  @override
  Widget build(BuildContext context) {
    final bodySize = HeroTokens.typeSm.resolve(context).fontSize;
    final muted = HeroTokens.colorMuted.resolve(context);

    return Padding(
      padding: EdgeInsets.all(HeroTokens.space2.resolve(context)), // p-2
      child: Align(
        alignment: alignment,
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            if (icon != null) ...[
              Icon(icon, size: bodySize, color: muted),
              SizedBox(height: HeroTokens.space1.resolve(context)),
            ],
            Text(
              message,
              style: TextStyle(fontSize: bodySize, color: muted),
            ),
          ],
        ),
      ),
    );
  }
}
