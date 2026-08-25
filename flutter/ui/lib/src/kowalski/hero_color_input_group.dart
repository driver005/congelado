import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';
import 'hero_color_swatch.dart';
import 'hero_focus_ring.dart';

/// HeroUI v3 color input group variants (color-input-group.css).
enum HeroColorInputGroupVariant {
  /// Field chrome (`.color-input-group--primary`): `bg-field shadow-field`.
  primary,

  /// Flat chrome (`.color-input-group--secondary`): `shadow-none`,
  /// default background, no focus background change.
  secondary,
}

final Map<(HeroColorInputGroupVariant, bool), RemixTextFieldStyle>
    _heroColorInputGroupStyleCache = {};

/// Returns the [RemixTextFieldStyle] for a HeroUI v3 color input group.
///
/// color-input-group.css: `.color-input-group` — `h-9` (36),
/// `rounded-field` (12), `border` + `--field-border`, `bg-field`,
/// `shadow-field`, `text-field-foreground`; hover `bg-field-hover` +
/// `field-border-hover`; invalid `status-invalid-field` + `bg-field-focus`;
/// the 2px accent ring (focus-within) is rendered by the facade's
/// [HeroFocusRing]. Secondary: `shadow-none` + default background.
RemixTextFieldStyle heroColorInputGroupStyle({
  HeroColorInputGroupVariant variant = HeroColorInputGroupVariant.primary,
  bool error = false,
}) {
  return _heroColorInputGroupStyleCache.putIfAbsent((variant, error), () {
    final flat = variant == HeroColorInputGroupVariant.secondary;
    var style = RemixTextFieldStyle(
      animation: AnimationConfig.curve(
        duration: const Duration(milliseconds: heroInputTransitionMs),
        curve: heroEaseSmooth,
      ),
    )
        .height(HeroTokens.doubleInputMinHeight())
        .borderRadius(
          BorderRadiusGeometryMix.all(HeroTokens.radiusField()),
        )
        .padding(
          EdgeInsetsGeometryMix.symmetric(
            horizontal: HeroTokens.doubleInputPaddingX(),
            vertical: HeroTokens.doubleInputPaddingY(),
          ),
        )
        .color(HeroTokens.colorFieldForeground())
        .hintColor(HeroTokens.colorFieldPlaceholder())
        .text(TextStyler().fontSize(HeroTokens.doubleInputFontSize()))
        .hintText(TextStyler().fontSize(HeroTokens.doubleInputFontSize()))
        .backgroundColor(
          flat ? HeroTokens.colorDefault() : HeroTokens.colorField(),
        )
        .borderAll(
          color: HeroTokens.colorFieldBorder(),
          width: HeroTokens.doubleBorderWidth(),
        );

    style = style.onHovered(
      RemixTextFieldStyle()
          .backgroundColor(
            flat
                ? HeroTokens.colorDefaultHover()
                : HeroTokens.colorFieldHover(),
          )
          .borderAll(
            color: HeroTokens.colorFieldBorderHover(),
            width: HeroTokens.doubleBorderWidth(),
          ),
    );

    if (error) {
      style = style.variant(
        ContextVariant.widgetState(WidgetState.error),
        RemixTextFieldStyle()
            .borderAll(
              color: HeroTokens.colorDanger(),
              width: HeroTokens.doubleBorderWidth(),
            )
            .backgroundColor(HeroTokens.colorFieldFocus()),
      );
    }

    return style;
  });
}

/// A HeroUI v3 color input group (color-input-group.css) — a compact hex
/// input with a leading color swatch that can open a picker.
///
/// ```dart
/// HeroColorInputGroup(
///   value: color,
///   onChanged: (c) => setState(() => color = c),
///   onPick: () async {
///     final picked = await showHeroColorPicker(context, initialColor: color);
///     if (picked != null) setState(() => color = picked);
///   },
/// )
/// ```
class HeroColorInputGroup extends StatefulWidget {
  const HeroColorInputGroup({
    super.key,
    this.value,
    this.onChanged,
    this.onPick,
    this.enabled = true,
    this.error = false,
    this.variant = HeroColorInputGroupVariant.primary,
    this.fullWidth = false,
    this.hintText,
    this.suffix,
  });

  /// The color to display. Null renders an empty field.
  final Color? value;

  /// Called with the parsed hex color, or null when the field is cleared.
  final ValueChanged<Color?>? onChanged;

  /// When non-null the leading swatch behaves as a button (e.g. opening
  /// [showHeroColorPicker]) and calls this on tap.
  final VoidCallback? onPick;

  final bool enabled;
  final bool error;
  final HeroColorInputGroupVariant variant;

  /// `.color-input-group--full-width`.
  final bool fullWidth;

  final String? hintText;

  /// Optional trailing slot (`.color-input-group__suffix`).
  final Widget? suffix;

  @override
  State<HeroColorInputGroup> createState() => _HeroColorInputGroupState();
}

class _HeroColorInputGroupState extends State<HeroColorInputGroup> {
  late final TextEditingController _controller;
  Color? _lastValue;

  @override
  void initState() {
    super.initState();
    _lastValue = widget.value;
    _controller = TextEditingController(text: _hexOf(widget.value));
  }

  @override
  void didUpdateWidget(HeroColorInputGroup oldWidget) {
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
    var style = heroColorInputGroupStyle(
      variant: widget.variant,
      error: widget.error,
    );
    if (widget.variant == HeroColorInputGroupVariant.primary) {
      final shadows = HeroTokens.shadowField.resolve(context);
      if (shadows.isNotEmpty) {
        style = style.boxShadows([for (final s in shadows) BoxShadowMix.value(s)]);
      }
    }

    final swatch = HeroColorSwatch(
      color: widget.value,
      size: HeroColorSwatchSize.sm,
    );
    final Widget leading;
    if (widget.onPick != null) {
      leading = Padding(
        padding: const EdgeInsetsDirectional.only(end: 8),
        child: HeroFocusRing(
          radius: heroColorSwatchRadius(
            HeroColorSwatchSize.sm,
            HeroColorSwatchShape.circle,
          ),
          builder: (context, node, focused) => Focus(
            focusNode: node,
            onKeyEvent: (node, event) => _activateOnKey(event, widget.onPick!),
            child: GestureDetector(
              behavior: HitTestBehavior.opaque,
              onTap: widget.enabled ? widget.onPick : null,
              child: swatch,
            ),
          ),
        ),
      );
    } else {
      leading = Padding(
        padding: const EdgeInsetsDirectional.only(end: 8),
        child: swatch,
      );
    }

    final group = HeroFocusRing(
      radius: HeroTokens.radiusField.resolve(context).x,
      builder: (context, node, focused) => RemixTextField(
        controller: _controller,
        focusNode: node,
        style: style,
        enabled: widget.enabled,
        error: widget.error,
        hintText: widget.hintText ?? 'Enter a color',
        onChanged: _onTextChanged,
        leading: leading,
        trailing: widget.suffix,
      ),
    );

    return SizedBox(
      width: widget.fullWidth ? double.infinity : null,
      child: group,
    );
  }
}

KeyEventResult _activateOnKey(KeyEvent event, VoidCallback onTap) {
  if (event is! KeyDownEvent) return KeyEventResult.ignored;
  if (event.logicalKey == LogicalKeyboardKey.enter ||
      event.logicalKey == LogicalKeyboardKey.space) {
    onTap();
    return KeyEventResult.handled;
  }
  return KeyEventResult.ignored;
}
