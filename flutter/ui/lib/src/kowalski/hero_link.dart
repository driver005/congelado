import 'package:flutter/material.dart';

import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// A HeroUI v3 link (link.css `.link`).
///
/// link.css: `.link` — `inline-flex items-center rounded-xl font-medium
/// text-link no-underline`; the underline (1.5px, `muted/50`) appears on
/// hover and turns `muted` while pressed; focus shows the standard focus
/// ring; disabled fades the whole link.
class HeroLink extends StatefulWidget {
  const HeroLink({
    super.key,
    required this.label,
    this.onPressed,
    this.icon,
    this.disabled = false,
  });

  /// The link text.
  final String label;

  /// Invoked when the link is activated.
  final VoidCallback? onPressed;

  /// Optional leading icon.
  final Widget? icon;

  final bool disabled;

  @override
  State<HeroLink> createState() => _HeroLinkState();
}

class _HeroLinkState extends State<HeroLink> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final enabled = !widget.disabled && widget.onPressed != null;
    final color = HeroTokens.colorLink.resolve(context);
    final underline = _pressed
        ? HeroTokens.colorMuted.resolve(context)
        : HeroTokens.colorMuted.resolve(context).withValues(alpha: 0.5);
    final style = HeroTokens.typeSm.resolve(context).copyWith(
          color: color,
          fontWeight: HeroTokens.weightMedium.resolve(context),
          decoration: (_hovered || _pressed) && enabled
              ? TextDecoration.underline
              : TextDecoration.none,
          decorationColor: underline,
          decorationThickness: 1.5,
        );
    final opacity =
        widget.disabled ? HeroTokens.doubleDisabledOpacity.resolve(context) : 1.0;

    return HeroFocusRing(
      radius: HeroTokens.radiusXl.resolve(context).x,
      builder: (context, node, focused) => Opacity(
        opacity: opacity,
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
              child: Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  if (widget.icon != null) ...[
                    widget.icon!,
                    SizedBox(width: HeroTokens.space1.resolve(context)),
                  ],
                  Text(widget.label, style: style),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}
