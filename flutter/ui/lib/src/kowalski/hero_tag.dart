import 'package:flutter/material.dart';
import 'package:mix/mix.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// HeroUI v3 tag sizes (tag.css `.tag--sm/md/lg`).
enum HeroTagSize { sm, md, lg }

/// HeroUI v3 tag variants (tag.css `.tag--<variant>`).
enum HeroTagVariant { default_, surface }

/// A HeroUI v3 tag (tag.css `.tag`) — a selectable pill with optional remove
/// button, used for filter chips.
///
/// tag.css: `.tag` — `inline-flex items-center gap-1 rounded-xl font-medium`;
/// sizes `--sm` (text-xs px-2 py-0.5), default (text-xs px-2 py-1), `--lg`
/// (text-sm px-2.5 py-1.5, rounded-2xl); `--default` is `bg-default
/// text-default-foreground`; `--surface` is `bg-surface
/// text-surface-foreground`; `[data-selected]` uses the accent soft palette.
class HeroTag extends StatefulWidget {
  const HeroTag({
    super.key,
    required this.label,
    this.onPressed,
    this.onRemove,
    this.size = HeroTagSize.md,
    this.variant = HeroTagVariant.default_,
    this.selected = false,
    this.disabled = false,
  });

  /// The tag label.
  final String label;

  /// Invoked when the tag is activated (selection toggle).
  final VoidCallback? onPressed;

  /// When non-null, renders a remove button that invokes this callback.
  final VoidCallback? onRemove;

  final HeroTagSize size;
  final HeroTagVariant variant;
  final bool selected;
  final bool disabled;

  @override
  State<HeroTag> createState() => _HeroTagState();
}

class _HeroTagState extends State<HeroTag> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final enabled = !widget.disabled && widget.onPressed != null;
    final (paddingX, paddingY, fontSize, radius) = switch (widget.size) {
      HeroTagSize.sm => (
          HeroTokens.space2.resolve(context),
          2.0,
          HeroTokens.doubleChipFontSizeSm.resolve(context),
          HeroTokens.radiusXl.resolve(context).x,
        ),
      HeroTagSize.md => (
          HeroTokens.space2.resolve(context),
          4.0,
          HeroTokens.doubleChipFontSizeMd.resolve(context),
          HeroTokens.radiusXl.resolve(context).x,
        ),
      HeroTagSize.lg => (
          10.0,
          6.0,
          HeroTokens.doubleChipFontSizeLg.resolve(context),
          HeroTokens.radius2xl.resolve(context).x,
        ),
    };
    final (:background, :foreground) = _tagColors(widget.variant, widget.selected);
    final opacity =
        widget.disabled ? HeroTokens.doubleDisabledOpacity.resolve(context) : 1.0;

    final pill = Container(
      padding: EdgeInsets.symmetric(horizontal: paddingX, vertical: paddingY),
      decoration: BoxDecoration(
        color: _hovered && enabled && !widget.selected
            ? _hoverVariant(widget.variant)
            : background.resolve(context),
        borderRadius: BorderRadius.circular(radius),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          Flexible(
            child: Text(
              widget.label,
              overflow: TextOverflow.ellipsis,
              style: TextStyle(
                fontSize: fontSize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: foreground.resolve(context),
              ),
            ),
          ),
          if (widget.onRemove != null) ...[
            SizedBox(width: HeroTokens.space05.resolve(context)),
            InkWell(
              onTap: widget.disabled ? null : widget.onRemove,
              customBorder: const CircleBorder(),
              child: Padding(
                padding: EdgeInsets.all(2.0),
                child: Icon(
                  Icons.close_rounded,
                  size: 12,
                  color: foreground.resolve(context),
                ),
              ),
            ),
          ],
        ],
      ),
    );

    return HeroFocusRing(
      radius: radius,
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
              child: AnimatedScale(
                scale: _pressed ? 0.95 : 1.0,
                duration: HeroMotion.durationOf(
                  context,
                  const Duration(milliseconds: 100),
                ),
                curve: HeroMotion.smooth,
                child: pill,
              ),
            ),
          ),
        ),
      ),
    );
  }

  Color _hoverVariant(HeroTagVariant variant) => switch (variant) {
        HeroTagVariant.default_ => HeroTokens.colorDefaultHover.resolve(context),
        HeroTagVariant.surface => HeroTokens.colorSurfaceHover.resolve(context),
      };
}

({ColorToken background, ColorToken foreground}) _tagColors(
  HeroTagVariant variant,
  bool selected,
) {
  if (selected) {
    // `.tag:is([data-selected="true"])` — accent soft palette.
    return (
      background: HeroTokens.colorAccentSoft,
      foreground: HeroTokens.colorAccentSoftForeground,
    );
  }
  return switch (variant) {
    HeroTagVariant.default_ => (
        background: HeroTokens.colorDefault,
        foreground: HeroTokens.colorDefaultForeground,
      ),
    HeroTagVariant.surface => (
        background: HeroTokens.colorSurface,
        foreground: HeroTokens.colorSurfaceForeground,
      ),
  };
}
