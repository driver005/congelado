import 'package:flutter/material.dart';
import 'package:remix/remix.dart';

import '../foundation/hero_color_roles.dart';
import '../tokens/hero_tokens.dart';

/// HeroUI v3 chip color classes (chip.css `.chip--<color>`).
enum HeroChipColor { accent, default_, success, warning, danger }

/// HeroUI v3 chip variants (chip.css compound classes).
enum HeroChipVariant {
  /// Solid fill (`.chip--primary.chip--<color>`).
  solid,

  /// Soft tinted fill (`.chip--<color>.chip--soft`).
  soft,

  /// Transparent fill (`.chip--tertiary`).
  tertiary,
}

/// HeroUI v3 chip sizes (chip.css `.chip--sm/md/lg`).
enum HeroChipSize { sm, md, lg }

final Map<(HeroChipVariant, HeroChipColor, HeroChipSize), RemixBadgeStyle>
    _heroChipStyleCache = {};

/// Returns the [RemixBadgeStyle] for a HeroUI v3 chip.
///
/// chip.css: `.chip` — `rounded-2xl` (16), `gap-0.5` (2), `text-xs
/// leading-5 font-medium`; sizes `--sm` (px-1 py-0), default (px-2 py-0.5),
/// `--lg` (px-3 py-1 text-sm).
RemixBadgeStyle heroChipStyle({
  HeroChipVariant variant = HeroChipVariant.solid,
  HeroChipColor color = HeroChipColor.default_,
  HeroChipSize size = HeroChipSize.md,
}) {
  return _heroChipStyleCache.putIfAbsent((variant, color, size), () {
    final (:fill, :foreground) = _chipColors(variant, color);
    final (paddingX, paddingY, fontSize) = switch (size) {
      HeroChipSize.sm => (
          HeroTokens.doubleChipPaddingXSm(),
          HeroTokens.doubleChipPaddingYSm(),
          HeroTokens.doubleChipFontSizeSm(),
        ),
      HeroChipSize.md => (
          HeroTokens.doubleChipPaddingXMd(),
          HeroTokens.doubleChipPaddingYMd(),
          HeroTokens.doubleChipFontSizeMd(),
        ),
      HeroChipSize.lg => (
          HeroTokens.doubleChipPaddingXLg(),
          HeroTokens.doubleChipPaddingYLg(),
          HeroTokens.doubleChipFontSizeLg(),
        ),
    };

    return RemixBadgeStyle()
        .color(fill?.call() ?? HeroTokens.colorTransparent())
        .foregroundColor(foreground())
        .borderRadius(BorderRadiusGeometryMix.all(HeroTokens.radius2xl()))
        .padding(
          EdgeInsetsGeometryMix.symmetric(
            horizontal: paddingX,
            vertical: paddingY,
          ),
        )
        .constraints(BoxConstraintsMix(minHeight: 0.0));
  });
}

/// Maps the component enum to the shared role, then resolves fill/foreground
/// per variant — ONE color table across the whole design system.
HeroColor _chipRole(HeroChipColor color) => switch (color) {
      HeroChipColor.accent => HeroColor.accent,
      HeroChipColor.default_ => HeroColor.default_,
      HeroChipColor.success => HeroColor.success,
      HeroChipColor.warning => HeroColor.warning,
      HeroChipColor.danger => HeroColor.danger,
    };

({ColorToken? fill, ColorToken foreground}) _chipColors(
  HeroChipVariant variant,
  HeroChipColor color,
) {
  final t = heroColorTokens(_chipRole(color));
  return switch (variant) {
    HeroChipVariant.solid => (fill: t.fill, foreground: t.fillForeground),
    HeroChipVariant.soft => (fill: t.soft, foreground: t.softForeground),
    HeroChipVariant.tertiary => (fill: null, foreground: t.softForeground),
  };
}

/// A HeroUI v3 chip — a small pill with optional avatar/icon and close
/// button (chip.css). The label is rendered inside a custom row, so this is
/// a hand-written facade over the badge container recipe.
class HeroChip extends StatelessWidget {
  const HeroChip({
    super.key,
    required this.label,
    this.variant = HeroChipVariant.solid,
    this.color = HeroChipColor.default_,
    this.size = HeroChipSize.md,
    this.icon,
    this.avatar,
    this.onClose,
  });

  final String label;
  final HeroChipVariant variant;
  final HeroChipColor color;
  final HeroChipSize size;

  /// Optional leading icon.
  final IconData? icon;

  /// Optional leading avatar (e.g. a small `HeroAvatar`).
  final Widget? avatar;

  /// When non-null, renders a close button that invokes this callback.
  final VoidCallback? onClose;

  @override
  Widget build(BuildContext context) {
    final foreground = _chipColors(variant, color).foreground;
    final fontSize = switch (size) {
      HeroChipSize.sm => HeroTokens.doubleChipFontSizeSm.resolve(context),
      HeroChipSize.md => HeroTokens.doubleChipFontSizeMd.resolve(context),
      HeroChipSize.lg => HeroTokens.doubleChipFontSizeLg.resolve(context),
    };

    return RemixBadge(
      style: heroChipStyle(variant: variant, color: color, size: size),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: [
          if (avatar != null) ...[
            avatar!,
            const SizedBox(width: 2),
          ],
          if (icon != null) ...[
            Icon(icon, size: fontSize + 4, color: foreground.resolve(context)),
            const SizedBox(width: 2),
          ],
          Text(
            label,
            style: TextStyle(
              fontSize: fontSize,
              // chip.css: `text-xs leading-5` = 12px/20px line height.
              height: 20.0 / fontSize,
              fontWeight: HeroTokens.weightMedium.resolve(context),
              color: foreground.resolve(context),
            ),
          ),
          if (onClose != null) ...[
            const SizedBox(width: 2),
            InkWell(
              onTap: onClose,
              customBorder: const CircleBorder(),
              child: Icon(
                Icons.close_rounded,
                size: fontSize + 2,
                color: foreground.resolve(context),
              ),
            ),
          ],
        ],
      ),
    );
  }
}
