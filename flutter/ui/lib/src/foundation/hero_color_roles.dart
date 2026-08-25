import 'package:mix/mix.dart';

import '../tokens/hero_tokens.dart';

/// The shared status/emphasis color roles.
///
/// Every component that takes a color (Chip, Badge, Avatar, Progress,
/// Spinner, …) resolves its token pairs through this single table — one
/// source of truth, so a token change updates every component identically.
enum HeroColor {
  accent,
  default_,
  success,
  warning,
  danger;
}

/// The complete token pair set for one [HeroColor] role.
typedef HeroColorTokens = ({
  ColorToken fill,
  ColorToken fillForeground,
  ColorToken hover,
  ColorToken soft,
  ColorToken softForeground,
  ColorToken softHover,
});

/// Resolves the role tokens for [color] — used by every color-taking
/// component in the design system.
HeroColorTokens heroColorTokens(HeroColor color) {
  return switch (color) {
    HeroColor.accent => (
        fill: HeroTokens.colorAccent,
        fillForeground: HeroTokens.colorAccentForeground,
        hover: HeroTokens.colorAccentHover,
        soft: HeroTokens.colorAccentSoft,
        softForeground: HeroTokens.colorAccentSoftForeground,
        softHover: HeroTokens.colorAccentSoftHover,
      ),
    HeroColor.default_ => (
        fill: HeroTokens.colorDefault,
        fillForeground: HeroTokens.colorDefaultForeground,
        hover: HeroTokens.colorDefaultHover,
        soft: HeroTokens.colorDefaultSoft,
        softForeground: HeroTokens.colorDefaultSoftForeground,
        softHover: HeroTokens.colorDefaultSoftHover,
      ),
    HeroColor.success => (
        fill: HeroTokens.colorSuccess,
        fillForeground: HeroTokens.colorSuccessForeground,
        hover: HeroTokens.colorSuccessHover,
        soft: HeroTokens.colorSuccessSoft,
        softForeground: HeroTokens.colorSuccessSoftForeground,
        softHover: HeroTokens.colorSuccessSoftHover,
      ),
    HeroColor.warning => (
        fill: HeroTokens.colorWarning,
        fillForeground: HeroTokens.colorWarningForeground,
        hover: HeroTokens.colorWarningHover,
        soft: HeroTokens.colorWarningSoft,
        softForeground: HeroTokens.colorWarningSoftForeground,
        softHover: HeroTokens.colorWarningSoftHover,
      ),
    HeroColor.danger => (
        fill: HeroTokens.colorDanger,
        fillForeground: HeroTokens.colorDangerForeground,
        hover: HeroTokens.colorDangerHover,
        soft: HeroTokens.colorDangerSoft,
        softForeground: HeroTokens.colorDangerSoftForeground,
        softHover: HeroTokens.colorDangerSoftHover,
      ),
  };
}
