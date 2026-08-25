import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

final RemixRadioStyle _heroRadioStyle = RemixRadioStyle()
    .size(HeroTokens.doubleRadioSize(), HeroTokens.doubleRadioSize())
    .borderRadius(BorderRadiusGeometryMix.all(HeroTokens.radiusRadioRadius()))
    .fillColor(HeroTokens.colorField())
    .indicatorColor(HeroTokens.colorAccent())
    .indicator(
      BoxStyler()
          .size(HeroTokens.doubleRadioIndicatorSize(), HeroTokens.doubleRadioIndicatorSize())
          .shapeCircle(),
    );

/// Returns the [RemixRadioStyle] for a HeroUI v3 radio button.
///
/// radio.css: `.radio__control` — `size-4` (16), `rounded-lg` (8),
/// `bg-field shadow-field`; the selected indicator dot uses the accent color.
/// The field shadow is per-theme; [HeroRadio] resolves it from the scope.
RemixRadioStyle heroRadioStyle() => _heroRadioStyle;

/// A HeroUI v3 radio option (radio.css). Must sit inside a [HeroRadioGroup].
class HeroRadio<T> extends StatelessWidget {
  const HeroRadio({
    super.key,
    required this.value,
    this.enabled = true,
  });

  /// The value this radio represents within its [HeroRadioGroup].
  final T value;

  final bool enabled;

  @override
  Widget build(BuildContext context) {
    var style = heroRadioStyle();
    final shadows = HeroTokens.shadowField.resolve(context);
    if (shadows.isNotEmpty) {
      style = style.boxShadows([for (final s in shadows) BoxShadowMix.value(s)]);
    }
    return HeroFocusRing(
      radius: HeroTokens.radiusRadioRadius.resolve(context).x,
      builder: (context, node, focused) => RemixRadio<T>(
      value: value,
      enabled: enabled,
      focusNode: node,
      style: style,
      enableFeedback: false,
      ),
    );
  }
}

/// A HeroUI v3 radio group (radio.css).
///
/// ```dart
/// HeroRadioGroup<String>(
///   groupValue: selected,
///   onChanged: (v) => setState(() => selected = v),
///   child: Column(
///     children: [
///       HeroRadio(value: 'a'),
///       HeroRadio(value: 'b'),
///     ],
///   ),
/// )
/// ```
class HeroRadioGroup<T> extends StatelessWidget {
  const HeroRadioGroup({
    super.key,
    required this.groupValue,
    required this.onChanged,
    required this.child,
  });

  final T groupValue;
  final ValueChanged<T> onChanged;
  final Widget child;

  @override
  Widget build(BuildContext context) {
    return RemixRadioGroup<T>(
      groupValue: groupValue,
      onChanged: (value) {
        if (value != null) onChanged(value);
      },
      child: child,
    );
  }
}
