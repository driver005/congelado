import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../foundation/hero_motion.dart';
import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';
import 'hero_input.dart';

/// A HeroUI v3 time field (time-field.css `.time-field`) — a segmented
/// hour/minute/(AM/PM) input sharing the date-input-group anatomy
/// (date-input-group.css `.date-input-group`).
///
/// The field is `h-9 rounded-field border bg-field shadow-field` (hover
/// `bg-field-hover`, focus-within `status-focused-field`); segments are
/// `rounded-md px-0.5 text-end` — focused `bg-accent-soft
/// text-accent-soft-foreground`, placeholders `text-field-placeholder`,
/// literal separators `p-0 text-muted`; invalid segments are `text-danger`
/// (focused: `bg-danger-soft text-danger-soft-foreground`).
class HeroTimeField extends StatefulWidget {
  const HeroTimeField({
    super.key,
    this.value,
    this.onChanged,
    this.label,
    this.helperText,
    this.error = false,
    this.enabled = true,
    this.variant = HeroInputVariant.primary,
    this.hourCycle = 12,
    this.fullWidth = false,
  });

  /// The current time, or null for an empty (placeholder) field.
  final TimeOfDay? value;

  final ValueChanged<TimeOfDay>? onChanged;

  final String? label;
  final String? helperText;
  final bool error;
  final bool enabled;
  final HeroInputVariant variant;

  /// 12 (with AM/PM segment) or 24 hour cycle.
  final int hourCycle;

  final bool fullWidth;

  @override
  State<HeroTimeField> createState() => _HeroTimeFieldState();
}

class _HeroTimeFieldState extends State<HeroTimeField> {
  static const int _segmentHour = 0;
  static const int _segmentMinute = 1;
  static const int _segmentMeridiem = 2;

  int? _hour; // 0-23
  int? _minute; // 0-59
  bool _isAm = true;
  int _active = _segmentHour;
  bool _hovered = false;

  @override
  void initState() {
    super.initState();
    _syncFromValue();
  }

  @override
  void didUpdateWidget(HeroTimeField oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.value != widget.value) _syncFromValue();
  }

  void _syncFromValue() {
    final v = widget.value;
    _hour = v?.hour;
    _minute = v?.minute;
    _isAm = v == null || v.hour < 12;
  }

  void _emit() {
    final h = _hour;
    final m = _minute;
    if (h == null || m == null) return;
    widget.onChanged?.call(TimeOfDay(hour: h, minute: m));
  }

  int get _displayHour {
    final h = _hour;
    if (h == null) return 0;
    if (widget.hourCycle == 24) return h;
    final twelve = h % 12;
    return twelve == 0 ? 12 : twelve;
  }

  String _formatSegment(int value) => value.toString().padLeft(2, '0');

  void _handleDigit(int digit) {
    setState(() {
      if (_active == _segmentHour) {
        if (widget.hourCycle == 24) {
          final current = _hour;
          _hour = current == null
              ? digit
              : (current * 10 + digit).clamp(0, 23);
        } else {
          final currentDisplay = _hour == null ? null : _displayHour;
          final display = currentDisplay == null
              ? digit
              : (currentDisplay * 10 + digit).clamp(1, 12);
          _hour = _hourFromDisplay12(display);
        }
      } else if (_active == _segmentMinute) {
        final current = _minute;
        _minute = current == null ? digit : (current * 10 + digit).clamp(0, 59);
      } else {
        _isAm = digit % 2 == 0 ? !_isAm : _isAm;
        _hour = _hour ?? 12;
        if (!_isAm && _hour != null && _hour! < 12) _hour = _hour! + 12;
        if (_isAm && _hour != null && _hour! >= 12) _hour = _hour! - 12;
      }
    });
    _emit();
  }

  int _hourFromDisplay12(int display) =>
      display == 12 ? (_isAm ? 0 : 12) : (_isAm ? display : display + 12);

  void _adjust(int delta) {
    setState(() {
      if (_active == _segmentHour) {
        if (widget.hourCycle == 24) {
          final h = _hour ?? 0;
          _hour = (h + delta + 24) % 24;
        } else {
          final display = _displayHour;
          final next = display + delta;
          final clamped = next > 12 ? 1 : (next < 1 ? 12 : next);
          _hour = _hourFromDisplay12(clamped);
        }
      } else if (_active == _segmentMinute) {
        final m = _minute ?? 0;
        _minute = (m + delta) % 60;
        if (_minute! < 0) _minute = 59;
      } else {
        _isAm = !_isAm;
        final h = _hour ?? 12;
        _hour = _isAm ? (h >= 12 ? h - 12 : h) : (h < 12 ? h + 12 : h);
      }
    });
    _emit();
  }

  KeyEventResult _handleKey(KeyEvent event) {
    if (event is! KeyDownEvent) return KeyEventResult.ignored;
    final key = event.logicalKey;
    if (key.keyId >= LogicalKeyboardKey.digit0.keyId &&
        key.keyId <= LogicalKeyboardKey.digit9.keyId) {
      _handleDigit(key.keyId - LogicalKeyboardKey.digit0.keyId);
      return KeyEventResult.handled;
    }
    if (key == LogicalKeyboardKey.numpad0 || key == LogicalKeyboardKey.numpad1 ||
        key == LogicalKeyboardKey.numpad2 || key == LogicalKeyboardKey.numpad3 ||
        key == LogicalKeyboardKey.numpad4 || key == LogicalKeyboardKey.numpad5 ||
        key == LogicalKeyboardKey.numpad6 || key == LogicalKeyboardKey.numpad7 ||
        key == LogicalKeyboardKey.numpad8 || key == LogicalKeyboardKey.numpad9) {
      _handleDigit(key.keyId - LogicalKeyboardKey.numpad0.keyId);
      return KeyEventResult.handled;
    }
    switch (key) {
      case LogicalKeyboardKey.arrowUp:
        _adjust(1);
        return KeyEventResult.handled;
      case LogicalKeyboardKey.arrowDown:
        _adjust(-1);
        return KeyEventResult.handled;
      case LogicalKeyboardKey.backspace:
        setState(() {
          if (_active == _segmentHour) {
            _hour = null;
          } else if (_active == _segmentMinute) {
            _minute = null;
          } else {
            _isAm = true;
          }
        });
        return KeyEventResult.handled;
      case LogicalKeyboardKey.arrowRight:
      case LogicalKeyboardKey.tab:
        setState(() => _active = _nextSegment());
        return KeyEventResult.handled;
      case LogicalKeyboardKey.arrowLeft:
        setState(() => _active = _prevSegment());
        return KeyEventResult.handled;
      default:
        return KeyEventResult.ignored;
    }
  }

  int _nextSegment() =>
      _active == _segmentMeridiem ? _segmentHour : _active + 1;

  int _prevSegment() =>
      _active == _segmentHour ? _segmentMeridiem : _active - 1;

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
                child: Focus(
                  focusNode: node,
                  canRequestFocus: widget.enabled,
                  onKeyEvent: widget.enabled
                      ? (node, event) => _handleKey(event)
                      : null,
                  child: Padding(
                    padding: EdgeInsets.symmetric(
                      horizontal: HeroTokens.doubleInputPaddingX.resolve(context),
                    ),
                    child: Row(
                      mainAxisSize: MainAxisSize.min,
                      children: [
                        _HeroTimeSegment(
                          text: _hour == null
                              ? '--'
                              : _formatSegment(_displayHour),
                          placeholder: _hour == null,
                          active: _active == _segmentHour,
                          error: widget.error,
                          enabled: widget.enabled,
                          onTap: () {
                            setState(() => _active = _segmentHour);
                            node.requestFocus();
                          },
                        ),
                        const Text(
                          ':',
                          style: TextStyle(fontSize: 14),
                        ),
                        _HeroTimeSegment(
                          text: _minute == null
                              ? '--'
                              : _formatSegment(_minute!),
                          placeholder: _minute == null,
                          active: _active == _segmentMinute,
                          error: widget.error,
                          enabled: widget.enabled,
                          onTap: () {
                            setState(() => _active = _segmentMinute);
                            node.requestFocus();
                          },
                        ),
                        if (widget.hourCycle == 12) ...[
                          SizedBox(width: HeroTokens.space2.resolve(context)),
                          _HeroTimeSegment(
                            text: _isAm ? 'AM' : 'PM',
                            placeholder: false,
                            active: _active == _segmentMeridiem,
                            error: widget.error,
                            enabled: widget.enabled,
                            onTap: () {
                              setState(() => _active = _segmentMeridiem);
                              node.requestFocus();
                            },
                          ),
                        ],
                      ],
                    ),
                  ),
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

/// One editable segment (date-input-group.css `.date-input-group__segment`):
/// `inline-block rounded-md px-0.5 text-end text-nowrap`; focused
/// `bg-accent-soft text-accent-soft-foreground`; invalid `text-danger`
/// (focused: `bg-danger-soft text-danger-soft-foreground`).
class _HeroTimeSegment extends StatelessWidget {
  const _HeroTimeSegment({
    required this.text,
    required this.placeholder,
    required this.active,
    required this.error,
    required this.enabled,
    required this.onTap,
  });

  final String text;
  final bool placeholder;
  final bool active;
  final bool error;
  final bool enabled;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final Color background;
    final Color foreground;
    if (active && error) {
      background = HeroTokens.colorDangerSoft.resolve(context);
      foreground = HeroTokens.colorDangerSoftForeground.resolve(context);
    } else if (active) {
      background = HeroTokens.colorAccentSoft.resolve(context);
      foreground = HeroTokens.colorAccentSoftForeground.resolve(context);
    } else if (error) {
      background = HeroTokens.colorTransparent.resolve(context);
      foreground = HeroTokens.colorDanger.resolve(context);
    } else {
      background = HeroTokens.colorTransparent.resolve(context);
      foreground = placeholder
          ? HeroTokens.colorFieldPlaceholder.resolve(context)
          : HeroTokens.colorFieldForeground.resolve(context);
    }

    return MouseRegion(
      cursor: enabled ? SystemMouseCursors.click : SystemMouseCursors.basic,
      child: GestureDetector(
        behavior: HitTestBehavior.opaque,
        onTap: enabled ? onTap : null,
        child: AnimatedContainer(
          duration: HeroMotion.durationOf(
            context,
            const Duration(milliseconds: heroInputTransitionMs),
          ),
          curve: HeroMotion.smooth,
          padding: EdgeInsets.symmetric(
            horizontal: HeroTokens.space05.resolve(context),
          ),
          decoration: BoxDecoration(
            color: background,
            borderRadius: BorderRadius.circular(
              HeroTokens.radiusMd.resolve(context).x,
            ),
          ),
          child: Text(
            text,
            textAlign: TextAlign.end,
            style: TextStyle(
              fontSize: HeroTokens.doubleInputFontSize.resolve(context),
              fontWeight: HeroTokens.weightSemibold.resolve(context),
              color: foreground,
              fontFeatures: const [FontFeature.tabularFigures()],
            ),
          ),
        ),
      ),
    );
  }
}
