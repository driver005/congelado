import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_input.dart';

/// A HeroUI v3 OTP input (input-otp.css `.input-otp`) — a row of single
/// character boxes.
///
/// `.input-otp__slot` — `h-10 w-9.5` (40×38), `rounded-field`, `bg-field`,
/// `shadow-field`, `text-sm font-semibold`; hover `bg-field-hover`; active
/// `bg-field-focus` + `status-focused-field`; filled `bg-field-focus`.
/// `.input-otp__slot-value` — `text-lg` (18) with a 250ms `slot-value-in`
/// animation; `.input-otp__separator` — 2×6px `bg-separator` gap.
class HeroInputOtp extends StatefulWidget {
  const HeroInputOtp({
    super.key,
    this.length = 6,
    this.value = '',
    this.onChanged,
    this.error = false,
    this.enabled = true,
    this.variant = HeroInputVariant.primary,
    this.groupEvery,
  });

  /// Number of segments.
  final int length;

  /// The current digit string (only `0-9` are kept).
  final String value;

  final ValueChanged<String>? onChanged;

  /// Invalid state: slots get the `status-invalid-field` danger outline.
  final bool error;

  final bool enabled;

  final HeroInputVariant variant;

  /// Inserts a `.input-otp__separator` after every N slots (e.g. 3 for a
  /// phone-style grouping); null = no separators.
  final int? groupEvery;

  @override
  State<HeroInputOtp> createState() => _HeroInputOtpState();
}

class _HeroInputOtpState extends State<HeroInputOtp> {
  late final TextEditingController _controller;
  final FocusNode _focusNode = FocusNode();
  int _caret = 0;
  bool _focused = false;

  @override
  void initState() {
    super.initState();
    _controller = TextEditingController(text: widget.value);
    _focusNode.addListener(_onFocusChanged);
  }

  @override
  void didUpdateWidget(HeroInputOtp oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.value != widget.value && _controller.text != widget.value) {
      _controller.text = widget.value;
      _caret = widget.value.length.clamp(0, widget.length);
    }
  }

  @override
  void dispose() {
    _focusNode.removeListener(_onFocusChanged);
    _focusNode.dispose();
    _controller.dispose();
    super.dispose();
  }

  void _onFocusChanged() {
    if (_focusNode.hasFocus != _focused) {
      setState(() => _focused = _focusNode.hasFocus);
    }
  }

  void _handleChanged(String text) {
    final digits = text.replaceAll(RegExp(r'[^0-9]'), '');
    if (digits != text) {
      _controller.text = digits;
      _controller.selection = TextSelection.collapsed(offset: digits.length);
    }
    setState(() => _caret = _controller.selection.baseOffset);
    widget.onChanged?.call(_controller.text);
  }

  void _tapSlot(int index) {
    if (!widget.enabled) return;
    final clamped = index.clamp(0, _controller.text.length);
    _controller.selection = TextSelection.collapsed(offset: clamped);
    setState(() => _caret = clamped);
    _focusNode.requestFocus();
  }

  @override
  Widget build(BuildContext context) {
    final opacity =
        widget.enabled ? 1.0 : HeroTokens.doubleDisabledOpacity.resolve(context);
    final value = _controller.text;

    return Opacity(
      opacity: opacity,
      child: MouseRegion(
        cursor: widget.enabled ? SystemMouseCursors.text : SystemMouseCursors.basic,
        child: Stack(
          children: [
            Row(
              children: [
                for (var i = 0; i < widget.length; i++) ...[
                  if (i > 0 && widget.groupEvery != null && i % widget.groupEvery! == 0)
                    Padding(
                      padding: EdgeInsets.symmetric(
                        horizontal: HeroTokens.space05.resolve(context),
                      ),
                      child: Container(
                        width: 6,
                        height: 2,
                        decoration: BoxDecoration(
                          color: HeroTokens.colorSeparator.resolve(context),
                          borderRadius: BorderRadius.circular(
                            HeroTokens.radiusSm.resolve(context).x,
                          ),
                        ),
                      ),
                    ),
                  _HeroOtpSlot(
                    value: i < value.length ? value[i] : null,
                    active: _focused && _caret == i && i < value.length + 1,
                    caret: _focused && _caret == i,
                    error: widget.error,
                    enabled: widget.enabled,
                    variant: widget.variant,
                    onTap: () => _tapSlot(i),
                  ),
                ],
              ],
            ),
            // Invisible input owning focus + the on-screen keyboard. It sits
            // behind the slots (taps land on the slots) but receives the
            // digits the user types.
            Positioned.fill(
              child: IgnorePointer(
                child: Opacity(
                  opacity: 0,
                  child: TextField(
                    controller: _controller,
                    focusNode: _focusNode,
                    enabled: widget.enabled,
                    keyboardType: TextInputType.number,
                    inputFormatters: [
                      FilteringTextInputFormatter.digitsOnly,
                      LengthLimitingTextInputFormatter(widget.length),
                    ],
                    onChanged: _handleChanged,
                    decoration: const InputDecoration(
                      border: InputBorder.none,
                    ),
                  ),
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

/// A single OTP segment box.
class _HeroOtpSlot extends StatefulWidget {
  const _HeroOtpSlot({
    required this.value,
    required this.active,
    required this.caret,
    required this.error,
    required this.enabled,
    required this.variant,
    required this.onTap,
  });

  final String? value;
  final bool active;
  final bool caret;
  final bool error;
  final bool enabled;
  final HeroInputVariant variant;
  final VoidCallback onTap;

  @override
  State<_HeroOtpSlot> createState() => _HeroOtpSlotState();
}

class _HeroOtpSlotState extends State<_HeroOtpSlot>
    with SingleTickerProviderStateMixin {
  bool _hovered = false;
  late final AnimationController _blink;

  @override
  void initState() {
    super.initState();
    _blink = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 1000),
    )..repeat(reverse: true);
  }

  @override
  void dispose() {
    _blink.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final radius = HeroTokens.radiusField.resolve(context).x;
    final filled = widget.value != null;
    final showRing = widget.active;

    Color background;
    if (widget.variant == HeroInputVariant.secondary) {
      background = widget.active || filled
          ? HeroTokens.colorDefault.resolve(context)
          : _hovered
              ? HeroTokens.colorDefaultHover.resolve(context)
              : HeroTokens.colorDefault.resolve(context);
    } else {
      background = widget.active || filled
          ? HeroTokens.colorFieldFocus.resolve(context)
          : _hovered
              ? HeroTokens.colorFieldHover.resolve(context)
              : HeroTokens.colorField.resolve(context);
    }

    final slot = AnimatedContainer(
      duration: HeroMotion.durationOf(
        context,
        const Duration(milliseconds: heroInputTransitionMs),
      ),
      curve: HeroMotion.smooth,
      width: 38,
      height: 40,
      decoration: BoxDecoration(
        color: background,
        borderRadius: BorderRadius.circular(radius),
        border: Border.all(
          color: widget.error
              ? HeroTokens.colorDanger.resolve(context)
              : _hovered && !widget.active
                  ? HeroTokens.colorFieldBorderHover.resolve(context)
                  : HeroTokens.colorFieldBorder.resolve(context),
          width: widget.error ? 1 : HeroTokens.doubleBorderWidth.resolve(context),
        ),
        boxShadow: widget.variant == HeroInputVariant.primary
            ? HeroTokens.shadowField.resolve(context)
            : null,
      ),
      child: Stack(
        alignment: Alignment.center,
        children: [
          if (widget.value != null)
            _HeroOtpSlotValue(char: widget.value!),
          if (widget.caret)
            FadeTransition(
              opacity: _blink,
              child: Container(
                width: 2,
                height: 16,
                decoration: BoxDecoration(
                  color: HeroTokens.colorFieldPlaceholder.resolve(context),
                  borderRadius: BorderRadius.circular(
                    HeroTokens.radiusSm.resolve(context).x,
                  ),
                ),
              ),
            ),
        ],
      ),
    );

    return MouseRegion(
      cursor: widget.enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
      onEnter: widget.enabled ? (_) => setState(() => _hovered = true) : null,
      onExit: widget.enabled ? (_) => setState(() => _hovered = false) : null,
      child: GestureDetector(
        behavior: HitTestBehavior.opaque,
        onTap: widget.enabled ? widget.onTap : null,
        child: showRing
            ? Padding(
                padding: const EdgeInsets.all(2),
                child: DecoratedBox(
                  decoration: BoxDecoration(
                    border: Border.all(
                      color: HeroTokens.colorFocus.resolve(context),
                      width: 2,
                    ),
                    borderRadius: BorderRadius.circular(radius + 2),
                  ),
                  child: slot,
                ),
              )
            : slot,
      ),
    );
  }
}

/// The animated slot digit (`.input-otp__slot-value`): `text-lg leading-6
/// tracking-[-0.27px]` with the 250ms `slot-value-in` animation (fade in,
/// translateY 8px, scale 0.8 → 1).
class _HeroOtpSlotValue extends StatelessWidget {
  const _HeroOtpSlotValue({required this.char});

  final String char;

  @override
  Widget build(BuildContext context) {
    return AnimatedSwitcher(
      duration: HeroMotion.durationOf(
        context,
        const Duration(milliseconds: 250),
      ),
      switchInCurve: HeroMotion.smooth,
      transitionBuilder: (child, animation) {
        final offset = Tween<Offset>(
          begin: const Offset(0, 8 / 24),
          end: Offset.zero,
        ).animate(animation);
        return FadeTransition(
          opacity: animation,
          child: SlideTransition(
            position: offset,
            child: ScaleTransition(scale: animation, child: child),
          ),
        );
      },
      child: Text(
        char,
        key: ValueKey(char),
        style: TextStyle(
          fontSize: HeroTokens.typeLg.resolve(context).fontSize,
          height: 24 / 18,
          letterSpacing: -0.27,
          fontWeight: HeroTokens.weightSemibold.resolve(context),
          color: HeroTokens.colorFieldForeground.resolve(context),
        ),
      ),
    );
  }
}
