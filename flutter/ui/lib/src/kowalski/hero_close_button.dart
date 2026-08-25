import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// A HeroUI v3 close button (close-button.css `.close-button`) — a small
/// square icon button with an X, used in chips, modals and drawers.
///
/// close-button.css: `.close-button` — `size-6 rounded-xl bg-default
/// text-muted`; hover `bg-default-hover`; pressed scales to 0.93.
class HeroCloseButton extends StatefulWidget {
  const HeroCloseButton({super.key, this.onPressed, this.disabled = false});

  final VoidCallback? onPressed;
  final bool disabled;

  @override
  State<HeroCloseButton> createState() => _HeroCloseButtonState();
}

class _HeroCloseButtonState extends State<HeroCloseButton> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final enabled = !widget.disabled && widget.onPressed != null;
    final size = HeroTokens.space6.resolve(context);
    final opacity =
        widget.disabled ? HeroTokens.doubleDisabledOpacity.resolve(context) : 1.0;

    return HeroFocusRing(
      radius: HeroTokens.radiusXl.resolve(context).x,
      builder: (context, node, focused) => Opacity(
        opacity: opacity,
        child: AnimatedScale(
          scale: _pressed ? 0.93 : 1.0,
          duration: HeroMotion.durationOf(
            context,
            const Duration(milliseconds: 250),
          ),
          curve: heroEaseOutQuart,
          child: MouseRegion(
            cursor: enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
            onEnter: enabled ? (_) => setState(() => _hovered = true) : null,
            onExit: enabled ? (_) => setState(() => _hovered = false) : null,
            child: Focus(
              focusNode: node,
              child: GestureDetector(
                onTap: enabled ? widget.onPressed : null,
                onTapDown: enabled ? (_) => setState(() => _pressed = true) : null,
                onTapUp: enabled ? (_) => setState(() => _pressed = false) : null,
                onTapCancel: enabled ? () => setState(() => _pressed = false) : null,
                child: AnimatedContainer(
                  duration: HeroMotion.durationOf(
                    context,
                    const Duration(milliseconds: 100),
                  ),
                  curve: heroEaseOut,
                  width: size,
                  height: size,
                  decoration: BoxDecoration(
                    color: _hovered
                        ? HeroTokens.colorDefaultHover.resolve(context)
                        : HeroTokens.colorDefault.resolve(context),
                    borderRadius: BorderRadius.circular(
                      HeroTokens.radiusXl.resolve(context).x,
                    ),
                  ),
                  child: Icon(
                    Icons.close_rounded,
                    size: size / 1.5,
                    color: HeroTokens.colorMuted.resolve(context),
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
    );
  }
}
