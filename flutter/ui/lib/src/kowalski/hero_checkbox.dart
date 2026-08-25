import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../tokens/hero_tokens.dart';
import 'hero_focus_ring.dart';

final RemixCheckboxStyle _heroCheckboxStyle = RemixCheckboxStyle()
    .size(HeroTokens.doubleCheckboxSize(), HeroTokens.doubleCheckboxSize())
    .borderRadius(BorderRadiusGeometryMix.all(HeroTokens.radiusCheckboxRadius()))
    .fillColor(HeroTokens.colorField())
    .indicatorColor(HeroTokens.colorAccentForeground())
    .icon(IconStyler(size: HeroTokens.doubleCheckboxIconSize()))
    .onHovered(
      RemixCheckboxStyle()
          .border(
            BoxBorderMix.all(
              BorderSideMix(color: HeroTokens.colorFieldBorderHover()),
            ),
          ),
    )
    .onSelected(
      RemixCheckboxStyle().fillColor(HeroTokens.colorAccent()),
    );

/// Returns the [RemixCheckboxStyle] for a HeroUI v3 checkbox.
///
/// checkbox.css: `.checkbox__control` — `size-4` (16), `rounded-md` (6),
/// `bg-field` (`shadow-field`), checked `::before bg-accent`; checkmark icon
/// `size-2.5` (10) `text-accent-foreground`. The field shadow is per-theme;
/// [HeroCheckbox] resolves it from the scope.
RemixCheckboxStyle heroCheckboxStyle() => _heroCheckboxStyle;

/// A HeroUI v3 checkbox (checkbox.css).
class HeroCheckbox extends StatelessWidget {
  const HeroCheckbox({
    super.key,
    required this.selected,
    this.onChanged,
    this.enabled = true,
  });

  final bool selected;
  final ValueChanged<bool>? onChanged;
  final bool enabled;

  @override
  Widget build(BuildContext context) {
    var style = heroCheckboxStyle();
    final shadows = HeroTokens.shadowField.resolve(context);
    if (shadows.isNotEmpty) {
      style = style.boxShadows([for (final s in shadows) BoxShadowMix.value(s)]);
    }
    return HeroFocusRing(
      radius: HeroTokens.radiusCheckboxRadius.resolve(context).x,
      builder: (context, node, focused) => RemixCheckbox(
      selected: selected,
      enabled: enabled,
      focusNode: node,
      onChanged: onChanged == null
          ? null
          : (value) {
              if (value != null) onChanged!(value);
            },
      style: style,
      enableFeedback: false,
      ),
    );
  }
}
