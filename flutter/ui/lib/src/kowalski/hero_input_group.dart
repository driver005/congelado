import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_input.dart';

/// A HeroUI v3 input group (input-group.css `.input-group`) — a field-shaped
/// shell that joins one or more borderless inputs with optional prefix/suffix
/// slots.
///
/// `.input-group` — `inline-flex min-h-9 items-center rounded-field border
/// bg-field text-sm text-field-foreground shadow-field`; hover `bg-field-hover`
/// + `border-field-hover`; when a child input is focused the shell paints
/// `status-focused-field`; invalid shows the danger outline; disabled fades to
/// `--disabled-opacity`. The prefix/suffix slots are `px-3 text-field-placeholder`
/// with a 1px `field-border` inner edge.
class HeroInputGroup extends StatefulWidget {
  const HeroInputGroup({
    super.key,
    required this.children,
    this.prefix,
    this.suffix,
    this.error = false,
    this.enabled = true,
    this.variant = HeroInputVariant.primary,
    this.fullWidth = false,
  });

  /// The inputs to join (typically [HeroInput] widgets without adornments).
  final List<Widget> children;

  /// Optional leading slot (`.input-group__prefix`).
  final Widget? prefix;

  /// Optional trailing slot (`.input-group__suffix`).
  final Widget? suffix;

  final bool error;
  final bool enabled;
  final HeroInputVariant variant;
  final bool fullWidth;

  @override
  State<HeroInputGroup> createState() => _HeroInputGroupState();
}

class _HeroInputGroupState extends State<HeroInputGroup> {
  final FocusScopeNode _scope = FocusScopeNode();
  bool _focused = false;
  bool _hovered = false;

  @override
  void initState() {
    super.initState();
    _scope.addListener(_onFocusChanged);
  }

  @override
  void dispose() {
    _scope.removeListener(_onFocusChanged);
    _scope.dispose();
    super.dispose();
  }

  void _onFocusChanged() {
    if (_scope.hasFocus != _focused) {
      setState(() => _focused = _scope.hasFocus);
    }
  }

  @override
  Widget build(BuildContext context) {
    final radius = HeroTokens.radiusField.resolve(context).x;
    final opacity =
        widget.enabled ? 1.0 : HeroTokens.doubleDisabledOpacity.resolve(context);

    final field = Opacity(
      opacity: opacity,
      child: MouseRegion(
        cursor: widget.enabled ? SystemMouseCursors.text : SystemMouseCursors.basic,
        onEnter: widget.enabled ? (_) => setState(() => _hovered = true) : null,
        onExit: widget.enabled ? (_) => setState(() => _hovered = false) : null,
        child: AnimatedContainer(
          duration: HeroMotion.durationOf(
            context,
            const Duration(milliseconds: heroInputTransitionMs),
          ),
          curve: HeroMotion.smooth,
          constraints: BoxConstraints(
            minHeight: HeroTokens.doubleInputMinHeight.resolve(context),
          ),
          width: widget.fullWidth ? double.infinity : null,
          decoration: BoxDecoration(
            color: _groupColor(),
            borderRadius: BorderRadius.circular(radius),
            border: Border.all(
              color: widget.error
                  ? HeroTokens.colorDanger.resolve(context)
                  : _hovered && !_focused
                      ? HeroTokens.colorFieldBorderHover.resolve(context)
                      : HeroTokens.colorFieldBorder.resolve(context),
              width: HeroTokens.doubleBorderWidth.resolve(context),
            ),
            boxShadow: widget.variant == HeroInputVariant.primary
                ? HeroTokens.shadowField.resolve(context)
                : null,
          ),
          child: FocusScope(
            node: _scope,
            child: Row(
              children: [
                if (widget.prefix != null)
                  _HeroInputGroupSlot(
                    radius: radius,
                    leading: true,
                    child: widget.prefix!,
                  ),
                ...widget.children,
                if (widget.suffix != null)
                  _HeroInputGroupSlot(
                    radius: radius,
                    leading: false,
                    child: widget.suffix!,
                  ),
              ],
            ),
          ),
        ),
      ),
    );

    // The 2px accent ring (status-focused-field) is drawn by the shell when a
    // child is focused — same outside-ring treatment as [HeroFocusRing].
    return AnimatedContainer(
      duration: HeroMotion.durationOf(
        context,
        const Duration(milliseconds: heroInputTransitionMs),
      ),
      curve: HeroMotion.smooth,
      padding: EdgeInsets.all(_focused ? 2.0 : 0.0),
      decoration: _focused
          ? BoxDecoration(
              border: Border.all(
                color: HeroTokens.colorFocus.resolve(context),
                width: 2,
              ),
              borderRadius: BorderRadius.circular(radius + 2),
            )
          : null,
      child: field,
    );
  }

  Color _groupColor() {
    if (widget.variant == HeroInputVariant.secondary) {
      if (_focused) return HeroTokens.colorDefault.resolve(context);
      if (_hovered) return HeroTokens.colorDefaultHover.resolve(context);
      return HeroTokens.colorDefault.resolve(context);
    }
    if (_focused) return HeroTokens.colorFieldFocus.resolve(context);
    if (_hovered) return HeroTokens.colorFieldHover.resolve(context);
    return HeroTokens.colorField.resolve(context);
  }
}

/// The `.input-group__prefix` / `__suffix` slot: `px-3 text-field-placeholder`
/// with a 1px `field-border` on the inner edge.
class _HeroInputGroupSlot extends StatelessWidget {
  const _HeroInputGroupSlot({
    required this.child,
    required this.radius,
    required this.leading,
  });

  final Widget child;
  final double radius;
  final bool leading;

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: EdgeInsets.symmetric(
        horizontal: HeroTokens.space3.resolve(context),
      ),
      constraints: BoxConstraints(
        minHeight: HeroTokens.doubleInputMinHeight.resolve(context),
      ),
      alignment: Alignment.center,
      decoration: BoxDecoration(
        color: HeroTokens.colorTransparent.resolve(context),
        borderRadius: leading
            ? BorderRadius.only(
                topLeft: Radius.circular(radius),
                bottomLeft: Radius.circular(radius),
              )
            : BorderRadius.only(
                topRight: Radius.circular(radius),
                bottomRight: Radius.circular(radius),
              ),
        border: Border(
          top: BorderSide.none,
          bottom: BorderSide.none,
          left: leading
              ? BorderSide.none
              : BorderSide(
                  color: HeroTokens.colorFieldBorder.resolve(context),
                  width: HeroTokens.doubleBorderWidth.resolve(context),
                ),
          right: leading
              ? BorderSide(
                  color: HeroTokens.colorFieldBorder.resolve(context),
                  width: HeroTokens.doubleBorderWidth.resolve(context),
                )
              : BorderSide.none,
        ),
      ),
      child: DefaultTextStyle(
        style: TextStyle(
          fontSize: HeroTokens.doubleInputFontSize.resolve(context),
          color: HeroTokens.colorFieldPlaceholder.resolve(context),
        ),
        child: child,
      ),
    );
  }
}
