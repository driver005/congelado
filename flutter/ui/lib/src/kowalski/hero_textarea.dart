import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';
import 'hero_input.dart';

final Map<(HeroInputVariant, bool), RemixTextFieldStyle> _heroTextareaStyleCache = {};

/// Returns the [RemixTextFieldStyle] for a HeroUI v3 textarea.
///
/// textarea.css: `.textarea` — same anatomy as `.input` (rounded-field,
/// `px-3 py-2`, `text-base sm:text-sm`, `shadow-field`, border-width
/// `--border-width-field`, hover `bg-field-hover`, focus `status-focused-field`
/// + `bg-field-focus`, invalid) with `min-height: 38px` instead of the input's
/// fixed height. The field shadow is per-theme; [HeroTextarea] resolves it
/// from the scope.
RemixTextFieldStyle heroTextareaStyle({
  HeroInputVariant variant = HeroInputVariant.primary,
  bool error = false,
}) {
  return _heroTextareaStyleCache.putIfAbsent((variant, error), () {
    var style = RemixTextFieldStyle(
      animation: AnimationConfig.curve(
        duration: const Duration(milliseconds: heroInputTransitionMs),
        curve: heroEaseSmooth,
      ),
    )
        .backgroundColor(HeroTokens.colorField())
        .borderRadius(BorderRadiusGeometryMix.all(HeroTokens.radiusField()))
        .padding(
          EdgeInsetsGeometryMix.symmetric(
            horizontal: HeroTokens.doubleInputPaddingX(),
            vertical: HeroTokens.doubleInputPaddingY(),
          ),
        )
        .minHeight(38.0)
        .hintColor(HeroTokens.colorFieldPlaceholder())
        .text(
          TextStyler().fontSize(HeroTokens.doubleInputFontSize()),
        )
        .hintText(
          TextStyler().fontSize(HeroTokens.doubleInputFontSize()),
        );

    if (variant == HeroInputVariant.secondary) {
      style = style
          .color(HeroTokens.colorDefault())
          .backgroundColor(HeroTokens.colorDefault());
    }

    style = style.onHovered(
      RemixTextFieldStyle()
          .backgroundColor(HeroTokens.colorFieldHover())
          .borderAll(
            color: HeroTokens.colorFieldBorderHover(),
            width: HeroTokens.doubleBorderWidth(),
          ),
    );

    style = style.onFocused(
      RemixTextFieldStyle().backgroundColor(HeroTokens.colorFieldFocus()),
    );

    if (error) {
      style = style.variant(
        ContextVariant.widgetState(WidgetState.error),
        RemixTextFieldStyle().borderAll(
          color: HeroTokens.colorDanger(),
          width: HeroTokens.doubleBorderWidth(),
        ),
      );
    }

    return style;
  });
}

/// A HeroUI v3 textarea (textarea.css `.textarea`) — a multiline field with
/// the same label/helper/error anatomy as [HeroInput].
class HeroTextarea extends StatelessWidget {
  const HeroTextarea({
    super.key,
    this.controller,
    this.variant = HeroInputVariant.primary,
    this.placeholder,
    this.label,
    this.helperText,
    this.error = false,
    this.enabled = true,
    this.minLines = 3,
    this.maxLines = 6,
    this.onChanged,
    this.focusNode,
  });

  final TextEditingController? controller;
  final HeroInputVariant variant;
  final String? placeholder;
  final String? label;
  final String? helperText;
  final bool error;
  final bool enabled;
  final int minLines;
  final int maxLines;
  final ValueChanged<String>? onChanged;
  final FocusNode? focusNode;

  @override
  Widget build(BuildContext context) {
    var style = heroTextareaStyle(variant: variant, error: error);
    if (variant == HeroInputVariant.primary) {
      final shadows = HeroTokens.shadowField.resolve(context);
      if (shadows.isNotEmpty) {
        style = style.boxShadows([for (final s in shadows) BoxShadowMix.value(s)]);
      }
    }
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        if (label != null)
          Padding(
            padding: EdgeInsets.only(bottom: HeroTokens.space1.resolve(context)),
            child: Text(
              label!,
              style: TextStyle(
                fontSize: HeroTokens.typeSm.resolve(context).fontSize,
                fontWeight: HeroTokens.weightMedium.resolve(context),
                color: HeroTokens.colorForeground.resolve(context),
              ),
            ),
          ),
        HeroFocusRing(
          radius: HeroTokens.radiusField.resolve(context).x,
          builder: (context, node, focused) => RemixTextField(
            controller: controller,
            focusNode: node,
            style: style,
            hintText: placeholder,
            error: error,
            enabled: enabled,
            minLines: minLines,
            maxLines: maxLines,
            onChanged: onChanged,
          ),
        ),
        if (helperText != null)
          Padding(
            padding: EdgeInsets.only(top: HeroTokens.space1.resolve(context)),
            child: Text(
              helperText!,
              style: TextStyle(
                fontSize: HeroTokens.typeXs.resolve(context).fontSize,
                color: error
                    ? HeroTokens.colorDanger.resolve(context)
                    : HeroTokens.colorMuted.resolve(context),
              ),
            ),
          ),
      ],
    );
  }
}
