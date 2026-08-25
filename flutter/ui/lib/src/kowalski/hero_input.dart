import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

/// HeroUI v3 input variants (input.css `.input--primary/secondary`).
enum HeroInputVariant {
  /// Field on the page background with a field shadow (`.input--primary`).
  primary,

  /// Flat field on the default background, no shadow (`.input--secondary`).
  secondary,
}

final Map<(HeroInputVariant, bool), RemixTextFieldStyle> _heroInputStyleCache = {};

/// Returns the [RemixTextFieldStyle] for a HeroUI v3 input.
///
/// input.css: `.input` — `rounded-field` (12), `px-3 py-2`, `text-base
/// sm:text-sm`, `shadow-field`, `border-width: var(--field-border-width)` (0),
/// hover `bg-field-hover`, focus `status-focused-field` (2px accent ring) +
/// `bg-field-focus`, invalid `status-invalid-field` (danger ring).
RemixTextFieldStyle heroInputStyle({
  HeroInputVariant variant = HeroInputVariant.primary,
  bool error = false,
}) {
  return _heroInputStyleCache.putIfAbsent((variant, error), () {
    final base = RemixTextFieldStyle(
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
        .height(HeroTokens.doubleInputMinHeight())
        .hintColor(HeroTokens.colorFieldPlaceholder())
        .text(
          TextStyler().fontSize(HeroTokens.doubleInputFontSize()),
        )
        .hintText(
          TextStyler().fontSize(HeroTokens.doubleInputFontSize()),
        );

    // The field shadow is per-theme (none in dark); the HeroInput facade
    // resolves it from the scope and merges it onto the recipe.
    var style = variant == HeroInputVariant.secondary
        ? base.color(HeroTokens.colorDefault()).backgroundColor(HeroTokens.colorDefault())
        : base;

    // Hover (input.css): bg-field-hover + field-border-hover.
    style = style.onHovered(
      RemixTextFieldStyle()
          .backgroundColor(HeroTokens.colorFieldHover())
          .borderAll(
            color: HeroTokens.colorFieldBorderHover(),
            width: HeroTokens.doubleBorderWidth(),
          ),
    );

    // Focus: bg-field-focus + focused border color; the 2px accent ring is
    // rendered by the HeroFocusRing wrapper in the facade (ring-offset style).
    style = style.onFocused(
      RemixTextFieldStyle().backgroundColor(HeroTokens.colorFieldFocus()),
    );

    if (error) {
      // Invalid (input.css `status-invalid-field`): 1px danger outline.
      // RemixTextField folds `error: true` into the WidgetState.error state,
      // which the style resolves via this explicit state variant.
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

/// A HeroUI v3 input (input.css).
class HeroInput extends StatelessWidget {
  const HeroInput({
    super.key,
    this.controller,
    this.variant = HeroInputVariant.primary,
    this.hintText,
    this.label,
    this.helperText,
    this.error = false,
    this.enabled = true,
    this.obscureText = false,
    this.keyboardType,
    this.textInputAction,
    this.maxLines = 1,
    this.leading,
    this.trailing,
    this.onChanged,
    this.onSubmitted,
    this.focusNode,
  });

  final TextEditingController? controller;
  final HeroInputVariant variant;
  final String? hintText;
  final String? label;
  final String? helperText;
  final bool error;
  final bool enabled;
  final bool obscureText;
  final TextInputType? keyboardType;
  final TextInputAction? textInputAction;
  final int? maxLines;
  final Widget? leading;
  final Widget? trailing;
  final ValueChanged<String>? onChanged;
  final ValueChanged<String>? onSubmitted;
  final FocusNode? focusNode;

  @override
  Widget build(BuildContext context) {
    var style = heroInputStyle(variant: variant, error: error);
    if (variant == HeroInputVariant.primary) {
      final shadows = HeroTokens.shadowField.resolve(context);
      if (shadows.isNotEmpty) {
        style = style.boxShadows([for (final s in shadows) BoxShadowMix.value(s)]);
      }
    }
    // Label + helper sit OUTSIDE the focus ring — the accent ring must wrap
    // only the field box, not the whole label/helper column (RemixTextField
    // would render all three inside its own wrapper).
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        if (label != null)
          Padding(
            padding: const EdgeInsets.only(bottom: 4),
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
            hintText: hintText,
            error: error,
            enabled: enabled,
            obscureText: obscureText,
            keyboardType: keyboardType,
            textInputAction: textInputAction,
            maxLines: maxLines,
            leading: leading,
            trailing: trailing,
            onChanged: onChanged,
            onSubmitted: onSubmitted,
          ),
        ),
        if (helperText != null)
          Padding(
            padding: const EdgeInsets.only(top: 4),
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
