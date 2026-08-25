import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';
import 'hero_color_swatch.dart';
import 'hero_focus_ring.dart';
import 'hero_input.dart';

/// A HeroUI v3 color field (color-field.css) — a hex color text input built
/// on the shared input anatomy (input.css: `bg-field`, `rounded-field`,
/// `shadow-field`, focus ring).
///
/// The wrapper mirrors `.color-field` (`flex flex-col gap-1`, label `w-fit`)
/// and `.color-field--full-width`. Typing a `#RRGGBB` / `#RRGGBBAA` hex value
/// (or `RRGGBB`) reports the color; clearing the field reports null.
class HeroColorField extends StatefulWidget {
  const HeroColorField({
    super.key,
    this.value,
    this.onChanged,
    this.label,
    this.helperText,
    this.error = false,
    this.enabled = true,
    this.fullWidth = false,
    this.showSwatch = true,
    this.hintText,
  });

  /// The color to display. Null renders an empty field.
  final Color? value;

  /// Called with the parsed hex color, or null when the field is cleared.
  final ValueChanged<Color?>? onChanged;

  final String? label;
  final String? helperText;
  final bool error;
  final bool enabled;

  /// `.color-field--full-width`.
  final bool fullWidth;

  /// Shows a leading color swatch inside the field.
  final bool showSwatch;

  final String? hintText;

  @override
  State<HeroColorField> createState() => _HeroColorFieldState();
}

class _HeroColorFieldState extends State<HeroColorField> {
  late final TextEditingController _controller;
  Color? _lastValue;

  @override
  void initState() {
    super.initState();
    _lastValue = widget.value;
    _controller = TextEditingController(text: _hexOf(widget.value));
  }

  @override
  void didUpdateWidget(HeroColorField oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.value != _lastValue) {
      _lastValue = widget.value;
      final text = _hexOf(widget.value);
      if (_controller.text != text) {
        _controller.text = text;
      }
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  static String _hexOf(Color? color) {
    if (color == null) return '';
    final hex = color.toARGB32().toRadixString(16).padLeft(8, '0').toUpperCase();
    return '#$hex';
  }

  static Color? _parseHex(String text) {
    var t = text.trim().replaceFirst('#', '');
    if (t.length == 6) t = 'FF$t';
    if (t.length != 8) return null;
    final value = int.tryParse(t, radix: 16);
    if (value == null) return null;
    return Color(value);
  }

  void _onTextChanged(String text) {
    if (text.trim().isEmpty) {
      if (_lastValue != null) {
        _lastValue = null;
        widget.onChanged?.call(null);
      }
      return;
    }
    final parsed = _parseHex(text);
    if (parsed == null || parsed == _lastValue) return;
    _lastValue = parsed;
    widget.onChanged?.call(parsed);
  }

  @override
  Widget build(BuildContext context) {
    var style = heroInputStyle(error: widget.error);
    final shadows = HeroTokens.shadowField.resolve(context);
    if (shadows.isNotEmpty) {
      style = style.boxShadows([for (final s in shadows) BoxShadowMix.value(s)]);
    }

    final field = HeroFocusRing(
      radius: HeroTokens.radiusField.resolve(context).x,
      builder: (context, node, focused) => RemixTextField(
        controller: _controller,
        focusNode: node,
        style: style,
        enabled: widget.enabled,
        error: widget.error,
        hintText: widget.hintText ?? 'Enter a color',
        onChanged: _onTextChanged,
        leading: widget.showSwatch
            ? Padding(
                padding: const EdgeInsetsDirectional.only(end: 8),
                child: HeroColorSwatch(
                  color: widget.value,
                  size: HeroColorSwatchSize.sm,
                ),
              )
            : null,
      ),
    );

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        if (widget.label != null)
          Padding(
            padding: const EdgeInsets.only(bottom: 4), // gap-1
            child: Text(
              widget.label!,
              style: TextStyle(
                fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorForeground.resolve(context),
              ),
            ),
          ),
        SizedBox(
          width: widget.fullWidth ? double.infinity : null,
          child: field,
        ),
        if (widget.helperText != null)
          Padding(
            padding: const EdgeInsets.only(top: 4), // gap-1
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
}
