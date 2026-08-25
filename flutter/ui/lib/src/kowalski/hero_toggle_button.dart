import 'package:flutter/material.dart';
import 'package:mix/mix.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// HeroUI v3 toggle button sizes (toggle-button.css `.toggle-button--sm/md/lg`;
/// md values: 32 / 36 / 40).
enum HeroToggleButtonSize { sm, md, lg }

/// HeroUI v3 toggle button variants (`.toggle-button--default/ghost`).
enum HeroToggleButtonVariant {
  /// Default fill (`--toggle-button-bg: var(--default)`).
  default_,

  /// Transparent fill, `default` on hover, `default-foreground` text.
  ghost,
}

/// Carries the attached-group geometry (`.toggle-button-group .toggle-button`)
/// to nested [HeroToggleButton]s: joined members lose their border radius
/// except on the outer edges, keep no pressed scale, and use an inset focus
/// ring. Provided by [HeroToggleButtonGroup]; ignored outside a group.
class HeroToggleButtonGroupScope extends InheritedWidget {
  const HeroToggleButtonGroupScope({
    super.key,
    required this.orientation,
    required this.detached,
    required this.index,
    required this.count,
    required super.child,
  });

  final Axis orientation;
  final bool detached;
  final int index;
  final int count;

  static HeroToggleButtonGroupScope? maybeOf(BuildContext context) =>
      context.dependOnInheritedWidgetOfExactType<HeroToggleButtonGroupScope>();

  @override
  bool updateShouldNotify(HeroToggleButtonGroupScope oldWidget) =>
      orientation != oldWidget.orientation ||
      detached != oldWidget.detached ||
      index != oldWidget.index ||
      count != oldWidget.count;
}

/// A HeroUI v3 toggle button (toggle-button.css `.toggle-button`) — a
/// selectable pill button.
///
/// toggle-button.css: `h-10 w-fit rounded-3xl px-4 text-sm font-medium
/// md:h-9`; unselected `bg-default` (ghost: transparent, `bg-default` on
/// hover), selected `bg-accent-soft text-accent-soft-foreground`; hover
/// `default-hover` / `accent-soft-hover`; pressed scales to 0.98/0.97/0.96;
/// transitions `transform 250ms ease-smooth`, `background-color 100ms
/// ease-out`. Disabled fades to `--disabled-opacity`.
class HeroToggleButton extends StatefulWidget {
  const HeroToggleButton({
    super.key,
    required this.label,
    this.icon,
    this.selected = false,
    this.onPressed,
    this.size = HeroToggleButtonSize.md,
    this.variant = HeroToggleButtonVariant.default_,
    this.iconOnly = false,
    this.disabled = false,
  });

  /// The button label.
  final String label;

  /// Optional leading icon.
  final IconData? icon;

  /// Selected state: `bg-accent-soft text-accent-soft-foreground`.
  final bool selected;

  /// Invoked when the button is activated. When null the button is disabled.
  final VoidCallback? onPressed;

  final HeroToggleButtonSize size;
  final HeroToggleButtonVariant variant;

  /// Icon-only modifier: removes horizontal padding and pins the width.
  final bool iconOnly;

  final bool disabled;

  @override
  State<HeroToggleButton> createState() => _HeroToggleButtonState();
}

class _HeroToggleButtonState extends State<HeroToggleButton> {
  bool _hovered = false;
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final enabled = !widget.disabled && widget.onPressed != null;
    final scope = HeroToggleButtonGroupScope.maybeOf(context);
    final joined = scope != null && !scope.detached;

    final (height, paddingX, fontSize, pressScale) = switch (widget.size) {
      HeroToggleButtonSize.sm => (32.0, 12.0, 14.0, 0.98),
      HeroToggleButtonSize.md => (36.0, 16.0, 14.0, 0.97),
      HeroToggleButtonSize.lg => (40.0, 16.0, 16.0, 0.96),
    };
    final width = widget.iconOnly
        ? switch (widget.size) {
            HeroToggleButtonSize.sm => 32.0,
            HeroToggleButtonSize.md => 36.0,
            HeroToggleButtonSize.lg => 40.0,
          }
        : null;
    final iconSize = widget.size == HeroToggleButtonSize.sm ? 16.0 : 20.0;
    final opacity =
        widget.disabled ? HeroTokens.doubleDisabledOpacity.resolve(context) : 1.0;

    final (:background, :foreground) = _toggleButtonColors(
      widget.variant,
      widget.selected,
      _hovered && enabled,
    );

    final pill = AnimatedContainer(
      duration: HeroMotion.durationOf(
        context,
        const Duration(milliseconds: 100),
      ),
      curve: HeroMotion.out,
      height: height,
      width: width,
      padding: EdgeInsets.symmetric(
        horizontal: widget.iconOnly ? 0 : paddingX,
      ),
      decoration: BoxDecoration(
        color: background.resolve(context),
        borderRadius: joined
            ? _joinedRadius(scope)
            : BorderRadius.circular(24),
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          if (widget.icon != null) ...[
            Icon(widget.icon, size: iconSize, color: foreground.resolve(context)),
            if (!widget.iconOnly) SizedBox(width: HeroTokens.space2.resolve(context)),
          ],
          if (!widget.iconOnly)
            Text(
              widget.label,
              style: TextStyle(
                fontSize: fontSize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: foreground.resolve(context),
              ),
            ),
        ],
      ),
    );

    return HeroFocusRing(
      radius: 24,
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
                // `.toggle-button-group .toggle-button:active { transform: none }`.
                scale: _pressed && !joined ? pressScale : 1.0,
                duration: HeroMotion.durationOf(
                  context,
                  const Duration(milliseconds: 250),
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

  BorderRadius _joinedRadius(HeroToggleButtonGroupScope scope) {
    final first = scope.index == 0;
    final last = scope.index == scope.count - 1;
    const r24 = Radius.circular(24);
    const zero = Radius.circular(0);
    if (scope.orientation == Axis.horizontal) {
      return BorderRadius.only(
        topLeft: first ? r24 : zero,
        bottomLeft: first ? r24 : zero,
        topRight: last ? r24 : zero,
        bottomRight: last ? r24 : zero,
      );
    }
    return BorderRadius.only(
      topLeft: first ? r24 : zero,
      topRight: first ? r24 : zero,
      bottomLeft: last ? r24 : zero,
      bottomRight: last ? r24 : zero,
    );
  }
}

({ColorToken background, ColorToken foreground}) _toggleButtonColors(
  HeroToggleButtonVariant variant,
  bool selected,
  bool hovered,
) {
  if (selected) {
    return (
      background: hovered
          ? HeroTokens.colorAccentSoftHover
          : HeroTokens.colorAccentSoft,
      foreground: HeroTokens.colorAccentSoftForeground,
    );
  }
  return switch (variant) {
    HeroToggleButtonVariant.default_ => (
        background: hovered
            ? HeroTokens.colorDefaultHover
            : HeroTokens.colorDefault,
        foreground: HeroTokens.colorDefaultForeground,
      ),
    HeroToggleButtonVariant.ghost => (
        background: hovered
            ? HeroTokens.colorDefault
            : HeroTokens.colorTransparent,
        foreground: HeroTokens.colorDefaultForeground,
      ),
  };
}
