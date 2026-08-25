import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';

/// A HeroUI v3 field error (field-error.css `.field-error`) — an animated
/// error caption that collapses to zero height/opacity when hidden.
///
/// field-error.css: `.field-error` — `px-1 text-xs text-danger`; transitions
/// opacity 150ms and height 350ms; hidden state is `h-0 opacity-0`.
class HeroFieldError extends StatelessWidget {
  const HeroFieldError(this.message, {super.key, this.visible = true});

  /// The error text; when empty the field error collapses.
  final String message;

  /// Whether the error is currently shown.
  final bool visible;

  @override
  Widget build(BuildContext context) {
    final show = visible && message.isNotEmpty;
    return AnimatedSize(
      duration: const Duration(milliseconds: 350),
      curve: HeroMotion.smooth,
      alignment: Alignment.topLeft,
      child: AnimatedOpacity(
        duration: const Duration(milliseconds: 150),
        curve: HeroMotion.out,
        opacity: show ? 1 : 0,
        child: show
            ? Padding(
                padding: EdgeInsets.symmetric(
                  horizontal: HeroTokens.space1.resolve(context),
                ),
                child: Text(
                  message,
                  style: HeroTokens.typeXs.resolve(context).copyWith(
                        color: HeroTokens.colorDanger.resolve(context),
                      ),
                ),
              )
            : const SizedBox(height: 0),
      ),
    );
  }
}
