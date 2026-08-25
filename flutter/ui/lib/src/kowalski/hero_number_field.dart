import 'package:flutter/material.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';
import 'hero_input.dart';

/// A HeroUI v3 number field (number-field.css `.number-field`) — a field with
/// increment/decrement stepper buttons.
///
/// `.number-field__group` mirrors the input anatomy (rounded-field, `bg-field`,
/// `shadow-field`, hover `bg-field-hover`, focus-within `status-focused-field`)
/// as a 3-column grid: `40px 1fr 40px`. The stepper buttons are transparent,
/// bordered on their inner edge with `field-placeholder/15`, and pressed shows
/// `field-foreground/10` + `scale(0.97)`.
class HeroNumberField extends StatefulWidget {
  const HeroNumberField({
    super.key,
    this.value = 0,
    this.onChanged,
    this.step = 1,
    this.min,
    this.max,
    this.variant = HeroInputVariant.primary,
    this.label,
    this.helperText,
    this.error = false,
    this.enabled = true,
    this.fullWidth = false,
  });

  final double value;
  final ValueChanged<double>? onChanged;
  final double step;

  /// Inclusive lower bound; null = unbounded.
  final double? min;

  /// Inclusive upper bound; null = unbounded.
  final double? max;

  final HeroInputVariant variant;
  final String? label;
  final String? helperText;
  final bool error;
  final bool enabled;
  final bool fullWidth;

  @override
  State<HeroNumberField> createState() => _HeroNumberFieldState();
}

class _HeroNumberFieldState extends State<HeroNumberField> {
  late final TextEditingController _controller;
  bool _hovered = false;

  @override
  void initState() {
    super.initState();
    _controller = TextEditingController(text: _format(widget.value));
  }

  @override
  void didUpdateWidget(HeroNumberField oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.value != widget.value) {
      final current = double.tryParse(_controller.text);
      if (current != widget.value) {
        _controller.text = _format(widget.value);
      }
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  String _format(double v) {
    if (v == v.roundToDouble()) return v.toInt().toString();
    return v.toString();
  }

  void _commit(double next) {
    final clamped = _clamp(next);
    _controller.text = _format(clamped);
    widget.onChanged?.call(clamped);
  }

  double _clamp(double v) {
    var result = v;
    if (widget.min != null && result < widget.min!) result = widget.min!;
    if (widget.max != null && result > widget.max!) result = widget.max!;
    return result;
  }

  void _handleInput(String text) {
    final parsed = double.tryParse(text.trim());
    if (parsed != null) {
      widget.onChanged?.call(_clamp(parsed));
    }
  }

  @override
  Widget build(BuildContext context) {
    final radius = HeroTokens.radiusField.resolve(context).x;
    final opacity =
        widget.enabled ? 1.0 : HeroTokens.doubleDisabledOpacity.resolve(context);

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        if (widget.label != null)
          Padding(
            padding: EdgeInsets.only(bottom: HeroTokens.space1.resolve(context)),
            child: Text(
              widget.label!,
              style: TextStyle(
                fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorForeground.resolve(context),
              ),
            ),
          ),
        HeroFocusRing(
          radius: radius,
          builder: (context, node, focused) => MouseRegion(
            cursor: widget.enabled
                ? SystemMouseCursors.text
                : SystemMouseCursors.basic,
            onEnter: widget.enabled ? (_) => setState(() => _hovered = true) : null,
            onExit: widget.enabled ? (_) => setState(() => _hovered = false) : null,
            child: Opacity(
              opacity: opacity,
              child: AnimatedContainer(
                duration: HeroMotion.durationOf(
                  context,
                  const Duration(milliseconds: heroInputTransitionMs),
                ),
                curve: HeroMotion.smooth,
                height: HeroTokens.doubleInputMinHeight.resolve(context),
                width: widget.fullWidth ? double.infinity : null,
                decoration: BoxDecoration(
                  color: _groupColor(focused),
                  borderRadius: BorderRadius.circular(radius),
                  border: Border.all(
                    color: widget.error
                        ? HeroTokens.colorDanger.resolve(context)
                        : _hovered && !focused
                            ? HeroTokens.colorFieldBorderHover.resolve(context)
                            : HeroTokens.colorFieldBorder.resolve(context),
                    width: HeroTokens.doubleBorderWidth.resolve(context),
                  ),
                  boxShadow: widget.variant == HeroInputVariant.primary
                      ? HeroTokens.shadowField.resolve(context)
                      : null,
                ),
                child: Row(
                  children: [
                    _HeroStepperButton(
                      icon: Icons.remove_rounded,
                      side: _StepperSide.decrement,
                      enabled: widget.enabled,
                      onPressed: () => _commit(_clamp(widget.value) - widget.step),
                    ),
                    Expanded(
                      child: TextField(
                        controller: _controller,
                        focusNode: node,
                        enabled: widget.enabled,
                        keyboardType: TextInputType.number,
                        textAlign: TextAlign.center,
                        style: TextStyle(
                          fontSize: HeroTokens.doubleInputFontSize.resolve(context),
                          color: HeroTokens.colorFieldForeground.resolve(context),
                          fontFeatures: const [FontFeature.tabularFigures()],
                        ),
                        decoration: const InputDecoration(
                          isCollapsed: true,
                          border: InputBorder.none,
                        ),
                        onChanged: _handleInput,
                      ),
                    ),
                    _HeroStepperButton(
                      icon: Icons.add_rounded,
                      side: _StepperSide.increment,
                      enabled: widget.enabled,
                      onPressed: () => _commit(_clamp(widget.value) + widget.step),
                    ),
                  ],
                ),
              ),
            ),
          ),
        ),
        if (widget.helperText != null)
          Padding(
            padding: EdgeInsets.only(top: HeroTokens.space1.resolve(context)),
            child: Text(
              widget.helperText!,
              style: TextStyle(
                fontSize: HeroTokens.typeXs.resolve(context).fontSize,
                color: widget.error
                    ? HeroTokens.colorDanger.resolve(context)
                    : HeroTokens.colorMuted.resolve(context),
              ),
            ),
          ),
      ],
    );
  }

  Color _groupColor(bool focused) {
    if (widget.variant == HeroInputVariant.secondary) {
      if (focused) return HeroTokens.colorDefault.resolve(context);
      if (_hovered) return HeroTokens.colorDefaultHover.resolve(context);
      return HeroTokens.colorDefault.resolve(context);
    }
    if (focused) return HeroTokens.colorFieldFocus.resolve(context);
    if (_hovered) return HeroTokens.colorFieldHover.resolve(context);
    return HeroTokens.colorField.resolve(context);
  }
}

enum _StepperSide { increment, decrement }

/// A number-field stepper button (`.number-field__increment-button` /
/// `__decrement-button`) — `w-10 h-full`, transparent, `text-field-foreground`,
/// inner border `field-placeholder/15`, pressed `field-foreground/10` +
/// `scale(0.97)`, icon `size-4`.
class _HeroStepperButton extends StatefulWidget {
  const _HeroStepperButton({
    required this.icon,
    required this.side,
    required this.enabled,
    required this.onPressed,
  });

  final IconData icon;
  final _StepperSide side;
  final bool enabled;
  final VoidCallback onPressed;

  @override
  State<_HeroStepperButton> createState() => _HeroStepperButtonState();
}

class _HeroStepperButtonState extends State<_HeroStepperButton> {
  bool _pressed = false;

  @override
  Widget build(BuildContext context) {
    final dividerColor =
        HeroTokens.colorFieldPlaceholder.resolve(context).withValues(alpha: 0.15);
    return MouseRegion(
      cursor: widget.enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
      child: GestureDetector(
        onTap: widget.enabled ? widget.onPressed : null,
        onTapDown: widget.enabled ? (_) => setState(() => _pressed = true) : null,
        onTapUp: widget.enabled ? (_) => setState(() => _pressed = false) : null,
        onTapCancel: widget.enabled ? () => setState(() => _pressed = false) : null,
        child: AnimatedScale(
          scale: _pressed ? 0.97 : 1.0,
          duration: HeroMotion.durationOf(
            context,
            const Duration(milliseconds: 100),
          ),
          curve: HeroMotion.out,
          child: AnimatedContainer(
            duration: HeroMotion.durationOf(
              context,
              const Duration(milliseconds: heroInputTransitionMs),
            ),
            curve: HeroMotion.smooth,
            width: 40,
            height: HeroTokens.doubleInputMinHeight.resolve(context),
            decoration: BoxDecoration(
              color: _pressed
                  ? HeroTokens.colorFieldForeground
                      .resolve(context)
                      .withValues(alpha: 0.1)
                  : HeroTokens.colorTransparent.resolve(context),
              border: Border(
                top: BorderSide.none,
                bottom: BorderSide.none,
                left: widget.side == _StepperSide.increment
                    ? BorderSide(color: dividerColor, width: 1)
                    : BorderSide.none,
                right: widget.side == _StepperSide.decrement
                    ? BorderSide(color: dividerColor, width: 1)
                    : BorderSide.none,
              ),
            ),
            child: Icon(
              widget.icon,
              size: 16,
              color: HeroTokens.colorFieldForeground.resolve(context),
            ),
          ),
        ),
      ),
    );
  }
}
