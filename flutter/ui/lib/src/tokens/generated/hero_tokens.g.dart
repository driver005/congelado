// GENERATED FILE — DO NOT EDIT BY HAND.
//
// Source     : @heroui/styles@3.2.4 (npm registry)
// Registry   : https://registry.npmjs.org/@heroui/styles
// Retrieved  : 2026-03-10
// Pinned CSS : tool/sources/*.css (see tool/sources/SHA256SUMS)
//
// Regenerate : python3 tool/normalize_tokens.py && python3 tool/generate_tokens.py
// Verify     : python3 tool/verify_generated.py
//
// SPDX-License-Identifier: MIT
//
// This file is generated from the committed normalized snapshot
// (tool/tokens/hero-tokens.normalized.json); regeneration is byte-identical.
//
// @dart=3.5

library;

import 'package:flutter/animation.dart'; // Cubic (not exported by painting)
import 'package:flutter/painting.dart';
import 'package:mix/mix.dart';

/// HeroUI v3 semantic token handles.
///
/// Call a token inside a styler chain to use its value as a Mix
/// reference (`HeroTokens.accent()`); resolve it with a BuildContext
/// to get the concrete value (`HeroTokens.accent.resolve(context)`).
/// Values live in the `HeroScope` token map (`buildHeroTokenMap`).
abstract final class HeroTokens {
  const HeroTokens._();

  /// `hero.shadow.field` (per-theme).
  static const BoxShadowToken shadowField = BoxShadowToken('hero.shadow.field');

  /// `hero.shadow.overlay` (per-theme).
  static const BoxShadowToken shadowOverlay = BoxShadowToken('hero.shadow.overlay');

  /// `hero.shadow.surface` (per-theme).
  static const BoxShadowToken shadowSurface = BoxShadowToken('hero.shadow.surface');

  /// `hero.color.accent` (per-theme).
  static const ColorToken colorAccent = ColorToken('hero.color.accent');

  /// `hero.color.accentForeground` (per-theme).
  static const ColorToken colorAccentForeground = ColorToken('hero.color.accentForeground');

  /// `hero.color.accentHover` (per-theme).
  static const ColorToken colorAccentHover = ColorToken('hero.color.accentHover');

  /// `hero.color.accentSoft` (per-theme).
  static const ColorToken colorAccentSoft = ColorToken('hero.color.accentSoft');

  /// `hero.color.accentSoftForeground` (per-theme).
  static const ColorToken colorAccentSoftForeground = ColorToken('hero.color.accentSoftForeground');

  /// `hero.color.accentSoftHover` (per-theme).
  static const ColorToken colorAccentSoftHover = ColorToken('hero.color.accentSoftHover');

  /// `hero.color.backdrop` (per-theme).
  static const ColorToken colorBackdrop = ColorToken('hero.color.backdrop');

  /// `hero.color.background` (per-theme).
  static const ColorToken colorBackground = ColorToken('hero.color.background');

  /// `hero.color.backgroundInverse` (per-theme).
  static const ColorToken colorBackgroundInverse = ColorToken('hero.color.backgroundInverse');

  /// `hero.color.backgroundSecondary` (per-theme).
  static const ColorToken colorBackgroundSecondary = ColorToken('hero.color.backgroundSecondary');

  /// `hero.color.backgroundTertiary` (per-theme).
  static const ColorToken colorBackgroundTertiary = ColorToken('hero.color.backgroundTertiary');

  /// `hero.color.black` (per-theme).
  static const ColorToken colorBlack = ColorToken('hero.color.black');

  /// `hero.color.border` (per-theme).
  static const ColorToken colorBorder = ColorToken('hero.color.border');

  /// `hero.color.borderSecondary` (per-theme).
  static const ColorToken colorBorderSecondary = ColorToken('hero.color.borderSecondary');

  /// `hero.color.borderTertiary` (per-theme).
  static const ColorToken colorBorderTertiary = ColorToken('hero.color.borderTertiary');

  /// `hero.color.danger` (per-theme).
  static const ColorToken colorDanger = ColorToken('hero.color.danger');

  /// `hero.color.dangerForeground` (per-theme).
  static const ColorToken colorDangerForeground = ColorToken('hero.color.dangerForeground');

  /// `hero.color.dangerHover` (per-theme).
  static const ColorToken colorDangerHover = ColorToken('hero.color.dangerHover');

  /// `hero.color.dangerSoft` (per-theme).
  static const ColorToken colorDangerSoft = ColorToken('hero.color.dangerSoft');

  /// `hero.color.dangerSoftForeground` (per-theme).
  static const ColorToken colorDangerSoftForeground = ColorToken('hero.color.dangerSoftForeground');

  /// `hero.color.dangerSoftHover` (per-theme).
  static const ColorToken colorDangerSoftHover = ColorToken('hero.color.dangerSoftHover');

  /// `hero.color.default` (per-theme).
  static const ColorToken colorDefault = ColorToken('hero.color.default');

  /// `hero.color.defaultForeground` (per-theme).
  static const ColorToken colorDefaultForeground = ColorToken('hero.color.defaultForeground');

  /// `hero.color.defaultHover` (per-theme).
  static const ColorToken colorDefaultHover = ColorToken('hero.color.defaultHover');

  /// `hero.color.defaultSoft` (per-theme).
  static const ColorToken colorDefaultSoft = ColorToken('hero.color.defaultSoft');

  /// `hero.color.defaultSoftForeground` (per-theme).
  static const ColorToken colorDefaultSoftForeground = ColorToken('hero.color.defaultSoftForeground');

  /// `hero.color.defaultSoftHover` (per-theme).
  static const ColorToken colorDefaultSoftHover = ColorToken('hero.color.defaultSoftHover');

  /// `hero.color.eclipse` (per-theme).
  static const ColorToken colorEclipse = ColorToken('hero.color.eclipse');

  /// `hero.color.field` (per-theme).
  static const ColorToken colorField = ColorToken('hero.color.field');

  /// `hero.color.fieldBorder` (per-theme).
  static const ColorToken colorFieldBorder = ColorToken('hero.color.fieldBorder');

  /// `hero.color.fieldBorderFocus` (per-theme).
  static const ColorToken colorFieldBorderFocus = ColorToken('hero.color.fieldBorderFocus');

  /// `hero.color.fieldBorderHover` (per-theme).
  static const ColorToken colorFieldBorderHover = ColorToken('hero.color.fieldBorderHover');

  /// `hero.color.fieldFocus` (per-theme).
  static const ColorToken colorFieldFocus = ColorToken('hero.color.fieldFocus');

  /// `hero.color.fieldForeground` (per-theme).
  static const ColorToken colorFieldForeground = ColorToken('hero.color.fieldForeground');

  /// `hero.color.fieldHover` (per-theme).
  static const ColorToken colorFieldHover = ColorToken('hero.color.fieldHover');

  /// `hero.color.fieldPlaceholder` (per-theme).
  static const ColorToken colorFieldPlaceholder = ColorToken('hero.color.fieldPlaceholder');

  /// `hero.color.focus` (per-theme).
  static const ColorToken colorFocus = ColorToken('hero.color.focus');

  /// `hero.color.foreground` (per-theme).
  static const ColorToken colorForeground = ColorToken('hero.color.foreground');

  /// `hero.color.link` (per-theme).
  static const ColorToken colorLink = ColorToken('hero.color.link');

  /// `hero.color.muted` (per-theme).
  static const ColorToken colorMuted = ColorToken('hero.color.muted');

  /// `hero.color.outlineHover` (per-theme).
  static const ColorToken colorOutlineHover = ColorToken('hero.color.outlineHover');

  /// `hero.color.overlay` (per-theme).
  static const ColorToken colorOverlay = ColorToken('hero.color.overlay');

  /// `hero.color.overlayForeground` (per-theme).
  static const ColorToken colorOverlayForeground = ColorToken('hero.color.overlayForeground');

  /// `hero.color.segment` (per-theme).
  static const ColorToken colorSegment = ColorToken('hero.color.segment');

  /// `hero.color.segmentForeground` (per-theme).
  static const ColorToken colorSegmentForeground = ColorToken('hero.color.segmentForeground');

  /// `hero.color.separator` (per-theme).
  static const ColorToken colorSeparator = ColorToken('hero.color.separator');

  /// `hero.color.separatorSecondary` (per-theme).
  static const ColorToken colorSeparatorSecondary = ColorToken('hero.color.separatorSecondary');

  /// `hero.color.separatorTertiary` (per-theme).
  static const ColorToken colorSeparatorTertiary = ColorToken('hero.color.separatorTertiary');

  /// `hero.color.snow` (per-theme).
  static const ColorToken colorSnow = ColorToken('hero.color.snow');

  /// `hero.color.success` (per-theme).
  static const ColorToken colorSuccess = ColorToken('hero.color.success');

  /// `hero.color.successForeground` (per-theme).
  static const ColorToken colorSuccessForeground = ColorToken('hero.color.successForeground');

  /// `hero.color.successHover` (per-theme).
  static const ColorToken colorSuccessHover = ColorToken('hero.color.successHover');

  /// `hero.color.successSoft` (per-theme).
  static const ColorToken colorSuccessSoft = ColorToken('hero.color.successSoft');

  /// `hero.color.successSoftForeground` (per-theme).
  static const ColorToken colorSuccessSoftForeground = ColorToken('hero.color.successSoftForeground');

  /// `hero.color.successSoftHover` (per-theme).
  static const ColorToken colorSuccessSoftHover = ColorToken('hero.color.successSoftHover');

  /// `hero.color.surface` (per-theme).
  static const ColorToken colorSurface = ColorToken('hero.color.surface');

  /// `hero.color.surfaceForeground` (per-theme).
  static const ColorToken colorSurfaceForeground = ColorToken('hero.color.surfaceForeground');

  /// `hero.color.surfaceHover` (per-theme).
  static const ColorToken colorSurfaceHover = ColorToken('hero.color.surfaceHover');

  /// `hero.color.surfaceSecondary` (per-theme).
  static const ColorToken colorSurfaceSecondary = ColorToken('hero.color.surfaceSecondary');

  /// `hero.color.surfaceSecondaryForeground` (per-theme).
  static const ColorToken colorSurfaceSecondaryForeground = ColorToken('hero.color.surfaceSecondaryForeground');

  /// `hero.color.surfaceTertiary` (per-theme).
  static const ColorToken colorSurfaceTertiary = ColorToken('hero.color.surfaceTertiary');

  /// `hero.color.surfaceTertiaryForeground` (per-theme).
  static const ColorToken colorSurfaceTertiaryForeground = ColorToken('hero.color.surfaceTertiaryForeground');

  /// `hero.color.transparent` (per-theme).
  static const ColorToken colorTransparent = ColorToken('hero.color.transparent');

  /// `hero.color.warning` (per-theme).
  static const ColorToken colorWarning = ColorToken('hero.color.warning');

  /// `hero.color.warningForeground` (per-theme).
  static const ColorToken colorWarningForeground = ColorToken('hero.color.warningForeground');

  /// `hero.color.warningHover` (per-theme).
  static const ColorToken colorWarningHover = ColorToken('hero.color.warningHover');

  /// `hero.color.warningSoft` (per-theme).
  static const ColorToken colorWarningSoft = ColorToken('hero.color.warningSoft');

  /// `hero.color.warningSoftForeground` (per-theme).
  static const ColorToken colorWarningSoftForeground = ColorToken('hero.color.warningSoftForeground');

  /// `hero.color.warningSoftHover` (per-theme).
  static const ColorToken colorWarningSoftHover = ColorToken('hero.color.warningSoftHover');

  /// `hero.color.white` (per-theme).
  static const ColorToken colorWhite = ColorToken('hero.color.white');

  /// `hero.double.avatar.fallbackFontSize.lg` (shared).
  static const DoubleToken doubleAvatarFallbackFontSizeLg = DoubleToken('hero.double.avatar.fallbackFontSize.lg');

  /// `hero.double.avatar.fallbackFontSize.md` (shared).
  static const DoubleToken doubleAvatarFallbackFontSizeMd = DoubleToken('hero.double.avatar.fallbackFontSize.md');

  /// `hero.double.avatar.fallbackFontSize.sm` (shared).
  static const DoubleToken doubleAvatarFallbackFontSizeSm = DoubleToken('hero.double.avatar.fallbackFontSize.sm');

  /// `hero.double.avatar.size.lg` (shared).
  static const DoubleToken doubleAvatarSizeLg = DoubleToken('hero.double.avatar.size.lg');

  /// `hero.double.avatar.size.md` (shared).
  static const DoubleToken doubleAvatarSizeMd = DoubleToken('hero.double.avatar.size.md');

  /// `hero.double.avatar.size.sm` (shared).
  static const DoubleToken doubleAvatarSizeSm = DoubleToken('hero.double.avatar.size.sm');

  /// `hero.double.badge.borderWidth` (shared).
  static const DoubleToken doubleBadgeBorderWidth = DoubleToken('hero.double.badge.borderWidth');

  /// `hero.double.badge.fontSize.lg` (shared).
  static const DoubleToken doubleBadgeFontSizeLg = DoubleToken('hero.double.badge.fontSize.lg');

  /// `hero.double.badge.fontSize.md` (shared).
  static const DoubleToken doubleBadgeFontSizeMd = DoubleToken('hero.double.badge.fontSize.md');

  /// `hero.double.badge.fontSize.sm` (shared).
  static const DoubleToken doubleBadgeFontSizeSm = DoubleToken('hero.double.badge.fontSize.sm');

  /// `hero.double.badge.gap` (shared).
  static const DoubleToken doubleBadgeGap = DoubleToken('hero.double.badge.gap');

  /// `hero.double.badge.minSize.lg` (shared).
  static const DoubleToken doubleBadgeMinSizeLg = DoubleToken('hero.double.badge.minSize.lg');

  /// `hero.double.badge.minSize.md` (shared).
  static const DoubleToken doubleBadgeMinSizeMd = DoubleToken('hero.double.badge.minSize.md');

  /// `hero.double.badge.minSize.sm` (shared).
  static const DoubleToken doubleBadgeMinSizeSm = DoubleToken('hero.double.badge.minSize.sm');

  /// `hero.double.badge.paddingX.lg` (shared).
  static const DoubleToken doubleBadgePaddingXLg = DoubleToken('hero.double.badge.paddingX.lg');

  /// `hero.double.badge.paddingX.md` (shared).
  static const DoubleToken doubleBadgePaddingXMd = DoubleToken('hero.double.badge.paddingX.md');

  /// `hero.double.badge.paddingX.sm` (shared).
  static const DoubleToken doubleBadgePaddingXSm = DoubleToken('hero.double.badge.paddingX.sm');

  /// `hero.double.badge.paddingY.lg` (shared).
  static const DoubleToken doubleBadgePaddingYLg = DoubleToken('hero.double.badge.paddingY.lg');

  /// `hero.double.badge.paddingY.md` (shared).
  static const DoubleToken doubleBadgePaddingYMd = DoubleToken('hero.double.badge.paddingY.md');

  /// `hero.double.badge.paddingY.sm` (shared).
  static const DoubleToken doubleBadgePaddingYSm = DoubleToken('hero.double.badge.paddingY.sm');

  /// `hero.double.borderWidth` (shared).
  static const DoubleToken doubleBorderWidth = DoubleToken('hero.double.borderWidth');

  /// `hero.double.button.focusRingOffset` (shared).
  static const DoubleToken doubleButtonFocusRingOffset = DoubleToken('hero.double.button.focusRingOffset');

  /// `hero.double.button.focusRingWidth` (shared).
  static const DoubleToken doubleButtonFocusRingWidth = DoubleToken('hero.double.button.focusRingWidth');

  /// `hero.double.button.fontSize.lg` (shared).
  static const DoubleToken doubleButtonFontSizeLg = DoubleToken('hero.double.button.fontSize.lg');

  /// `hero.double.button.fontSize.md` (shared).
  static const DoubleToken doubleButtonFontSizeMd = DoubleToken('hero.double.button.fontSize.md');

  /// `hero.double.button.fontSize.sm` (shared).
  static const DoubleToken doubleButtonFontSizeSm = DoubleToken('hero.double.button.fontSize.sm');

  /// `hero.double.button.gap` (shared).
  static const DoubleToken doubleButtonGap = DoubleToken('hero.double.button.gap');

  /// `hero.double.button.height.lg` (shared).
  static const DoubleToken doubleButtonHeightLg = DoubleToken('hero.double.button.height.lg');

  /// `hero.double.button.height.md` (shared).
  static const DoubleToken doubleButtonHeightMd = DoubleToken('hero.double.button.height.md');

  /// `hero.double.button.height.sm` (shared).
  static const DoubleToken doubleButtonHeightSm = DoubleToken('hero.double.button.height.sm');

  /// `hero.double.button.iconOnlyWidth.lg` (shared).
  static const DoubleToken doubleButtonIconOnlyWidthLg = DoubleToken('hero.double.button.iconOnlyWidth.lg');

  /// `hero.double.button.iconOnlyWidth.md` (shared).
  static const DoubleToken doubleButtonIconOnlyWidthMd = DoubleToken('hero.double.button.iconOnlyWidth.md');

  /// `hero.double.button.iconOnlyWidth.sm` (shared).
  static const DoubleToken doubleButtonIconOnlyWidthSm = DoubleToken('hero.double.button.iconOnlyWidth.sm');

  /// `hero.double.button.iconSize.lg` (shared).
  static const DoubleToken doubleButtonIconSizeLg = DoubleToken('hero.double.button.iconSize.lg');

  /// `hero.double.button.iconSize.md` (shared).
  static const DoubleToken doubleButtonIconSizeMd = DoubleToken('hero.double.button.iconSize.md');

  /// `hero.double.button.iconSize.sm` (shared).
  static const DoubleToken doubleButtonIconSizeSm = DoubleToken('hero.double.button.iconSize.sm');

  /// `hero.double.button.paddingX.lg` (shared).
  static const DoubleToken doubleButtonPaddingXLg = DoubleToken('hero.double.button.paddingX.lg');

  /// `hero.double.button.paddingX.md` (shared).
  static const DoubleToken doubleButtonPaddingXMd = DoubleToken('hero.double.button.paddingX.md');

  /// `hero.double.button.paddingX.sm` (shared).
  static const DoubleToken doubleButtonPaddingXSm = DoubleToken('hero.double.button.paddingX.sm');

  /// `hero.double.button.pressScale.lg` (shared).
  static const DoubleToken doubleButtonPressScaleLg = DoubleToken('hero.double.button.pressScale.lg');

  /// `hero.double.button.pressScale.md` (shared).
  static const DoubleToken doubleButtonPressScaleMd = DoubleToken('hero.double.button.pressScale.md');

  /// `hero.double.button.pressScale.sm` (shared).
  static const DoubleToken doubleButtonPressScaleSm = DoubleToken('hero.double.button.pressScale.sm');

  /// `hero.double.button.transitionBackgroundMs` (shared).
  static const DoubleToken doubleButtonTransitionBackgroundMs = DoubleToken('hero.double.button.transitionBackgroundMs');

  /// `hero.double.button.transitionTransformMs` (shared).
  static const DoubleToken doubleButtonTransitionTransformMs = DoubleToken('hero.double.button.transitionTransformMs');

  /// `hero.double.card.descriptionFontSize` (shared).
  static const DoubleToken doubleCardDescriptionFontSize = DoubleToken('hero.double.card.descriptionFontSize');

  /// `hero.double.card.descriptionLineHeight` (shared).
  static const DoubleToken doubleCardDescriptionLineHeight = DoubleToken('hero.double.card.descriptionLineHeight');

  /// `hero.double.card.gap` (shared).
  static const DoubleToken doubleCardGap = DoubleToken('hero.double.card.gap');

  /// `hero.double.card.padding` (shared).
  static const DoubleToken doubleCardPadding = DoubleToken('hero.double.card.padding');

  /// `hero.double.card.titleFontSize` (shared).
  static const DoubleToken doubleCardTitleFontSize = DoubleToken('hero.double.card.titleFontSize');

  /// `hero.double.card.titleLineHeight` (shared).
  static const DoubleToken doubleCardTitleLineHeight = DoubleToken('hero.double.card.titleLineHeight');

  /// `hero.double.checkbox.iconSize` (shared).
  static const DoubleToken doubleCheckboxIconSize = DoubleToken('hero.double.checkbox.iconSize');

  /// `hero.double.checkbox.size` (shared).
  static const DoubleToken doubleCheckboxSize = DoubleToken('hero.double.checkbox.size');

  /// `hero.double.chip.fontSize.lg` (shared).
  static const DoubleToken doubleChipFontSizeLg = DoubleToken('hero.double.chip.fontSize.lg');

  /// `hero.double.chip.fontSize.md` (shared).
  static const DoubleToken doubleChipFontSizeMd = DoubleToken('hero.double.chip.fontSize.md');

  /// `hero.double.chip.fontSize.sm` (shared).
  static const DoubleToken doubleChipFontSizeSm = DoubleToken('hero.double.chip.fontSize.sm');

  /// `hero.double.chip.gap` (shared).
  static const DoubleToken doubleChipGap = DoubleToken('hero.double.chip.gap');

  /// `hero.double.chip.paddingX.lg` (shared).
  static const DoubleToken doubleChipPaddingXLg = DoubleToken('hero.double.chip.paddingX.lg');

  /// `hero.double.chip.paddingX.md` (shared).
  static const DoubleToken doubleChipPaddingXMd = DoubleToken('hero.double.chip.paddingX.md');

  /// `hero.double.chip.paddingX.sm` (shared).
  static const DoubleToken doubleChipPaddingXSm = DoubleToken('hero.double.chip.paddingX.sm');

  /// `hero.double.chip.paddingY.lg` (shared).
  static const DoubleToken doubleChipPaddingYLg = DoubleToken('hero.double.chip.paddingY.lg');

  /// `hero.double.chip.paddingY.md` (shared).
  static const DoubleToken doubleChipPaddingYMd = DoubleToken('hero.double.chip.paddingY.md');

  /// `hero.double.chip.paddingY.sm` (shared).
  static const DoubleToken doubleChipPaddingYSm = DoubleToken('hero.double.chip.paddingY.sm');

  /// `hero.double.disabledOpacity` (shared).
  static const DoubleToken doubleDisabledOpacity = DoubleToken('hero.double.disabledOpacity');

  /// `hero.double.focusRingWidth` (shared).
  static const DoubleToken doubleFocusRingWidth = DoubleToken('hero.double.focusRingWidth');

  /// `hero.double.input.fontSize` (shared).
  static const DoubleToken doubleInputFontSize = DoubleToken('hero.double.input.fontSize');

  /// `hero.double.input.minHeight` (shared).
  static const DoubleToken doubleInputMinHeight = DoubleToken('hero.double.input.minHeight');

  /// `hero.double.input.paddingX` (shared).
  static const DoubleToken doubleInputPaddingX = DoubleToken('hero.double.input.paddingX');

  /// `hero.double.input.paddingY` (shared).
  static const DoubleToken doubleInputPaddingY = DoubleToken('hero.double.input.paddingY');

  /// `hero.double.input.transitionMs` (shared).
  static const DoubleToken doubleInputTransitionMs = DoubleToken('hero.double.input.transitionMs');

  /// `hero.double.modal.maxWidth.lg` (shared).
  static const DoubleToken doubleModalMaxWidthLg = DoubleToken('hero.double.modal.maxWidth.lg');

  /// `hero.double.modal.maxWidth.md` (shared).
  static const DoubleToken doubleModalMaxWidthMd = DoubleToken('hero.double.modal.maxWidth.md');

  /// `hero.double.modal.maxWidth.sm` (shared).
  static const DoubleToken doubleModalMaxWidthSm = DoubleToken('hero.double.modal.maxWidth.sm');

  /// `hero.double.modal.maxWidth.xs` (shared).
  static const DoubleToken doubleModalMaxWidthXs = DoubleToken('hero.double.modal.maxWidth.xs');

  /// `hero.double.modal.padding` (shared).
  static const DoubleToken doubleModalPadding = DoubleToken('hero.double.modal.padding');

  /// `hero.double.progress.fontSize` (shared).
  static const DoubleToken doubleProgressFontSize = DoubleToken('hero.double.progress.fontSize');

  /// `hero.double.progress.trackHeight` (shared).
  static const DoubleToken doubleProgressTrackHeight = DoubleToken('hero.double.progress.trackHeight');

  /// `hero.double.progress.transitionMs` (shared).
  static const DoubleToken doubleProgressTransitionMs = DoubleToken('hero.double.progress.transitionMs');

  /// `hero.double.radio.indicatorSize` (shared).
  static const DoubleToken doubleRadioIndicatorSize = DoubleToken('hero.double.radio.indicatorSize');

  /// `hero.double.radio.size` (shared).
  static const DoubleToken doubleRadioSize = DoubleToken('hero.double.radio.size');

  /// `hero.double.ringOffset` (shared).
  static const DoubleToken doubleRingOffset = DoubleToken('hero.double.ringOffset');

  /// `hero.double.separator.thickness` (shared).
  static const DoubleToken doubleSeparatorThickness = DoubleToken('hero.double.separator.thickness');

  /// `hero.double.skeleton.animationMs` (shared).
  static const DoubleToken doubleSkeletonAnimationMs = DoubleToken('hero.double.skeleton.animationMs');

  /// `hero.double.skeleton.opacity` (shared).
  static const DoubleToken doubleSkeletonOpacity = DoubleToken('hero.double.skeleton.opacity');

  /// `hero.double.spinner.animationMs` (shared).
  static const DoubleToken doubleSpinnerAnimationMs = DoubleToken('hero.double.spinner.animationMs');

  /// `hero.double.spinner.size.lg` (shared).
  static const DoubleToken doubleSpinnerSizeLg = DoubleToken('hero.double.spinner.size.lg');

  /// `hero.double.spinner.size.md` (shared).
  static const DoubleToken doubleSpinnerSizeMd = DoubleToken('hero.double.spinner.size.md');

  /// `hero.double.spinner.size.sm` (shared).
  static const DoubleToken doubleSpinnerSizeSm = DoubleToken('hero.double.spinner.size.sm');

  /// `hero.double.spinner.size.xl` (shared).
  static const DoubleToken doubleSpinnerSizeXl = DoubleToken('hero.double.spinner.size.xl');

  /// `hero.double.spinner.strokeWidth` (shared).
  static const DoubleToken doubleSpinnerStrokeWidth = DoubleToken('hero.double.spinner.strokeWidth');

  /// `hero.double.switch.controlHeight.lg` (shared).
  static const DoubleToken doubleSwitchControlHeightLg = DoubleToken('hero.double.switch.controlHeight.lg');

  /// `hero.double.switch.controlHeight.md` (shared).
  static const DoubleToken doubleSwitchControlHeightMd = DoubleToken('hero.double.switch.controlHeight.md');

  /// `hero.double.switch.controlHeight.sm` (shared).
  static const DoubleToken doubleSwitchControlHeightSm = DoubleToken('hero.double.switch.controlHeight.sm');

  /// `hero.double.switch.controlWidth.lg` (shared).
  static const DoubleToken doubleSwitchControlWidthLg = DoubleToken('hero.double.switch.controlWidth.lg');

  /// `hero.double.switch.controlWidth.md` (shared).
  static const DoubleToken doubleSwitchControlWidthMd = DoubleToken('hero.double.switch.controlWidth.md');

  /// `hero.double.switch.controlWidth.sm` (shared).
  static const DoubleToken doubleSwitchControlWidthSm = DoubleToken('hero.double.switch.controlWidth.sm');

  /// `hero.double.switch.thumbHeight.lg` (shared).
  static const DoubleToken doubleSwitchThumbHeightLg = DoubleToken('hero.double.switch.thumbHeight.lg');

  /// `hero.double.switch.thumbHeight.md` (shared).
  static const DoubleToken doubleSwitchThumbHeightMd = DoubleToken('hero.double.switch.thumbHeight.md');

  /// `hero.double.switch.thumbHeight.sm` (shared).
  static const DoubleToken doubleSwitchThumbHeightSm = DoubleToken('hero.double.switch.thumbHeight.sm');

  /// `hero.double.switch.thumbWidth.lg` (shared).
  static const DoubleToken doubleSwitchThumbWidthLg = DoubleToken('hero.double.switch.thumbWidth.lg');

  /// `hero.double.switch.thumbWidth.md` (shared).
  static const DoubleToken doubleSwitchThumbWidthMd = DoubleToken('hero.double.switch.thumbWidth.md');

  /// `hero.double.switch.thumbWidth.sm` (shared).
  static const DoubleToken doubleSwitchThumbWidthSm = DoubleToken('hero.double.switch.thumbWidth.sm');

  /// `hero.double.tabs.fontSize` (shared).
  static const DoubleToken doubleTabsFontSize = DoubleToken('hero.double.tabs.fontSize');

  /// `hero.double.tabs.indicatorHeight` (shared).
  static const DoubleToken doubleTabsIndicatorHeight = DoubleToken('hero.double.tabs.indicatorHeight');

  /// `hero.double.tabs.listPadding` (shared).
  static const DoubleToken doubleTabsListPadding = DoubleToken('hero.double.tabs.listPadding');

  /// `hero.double.tabs.panelGap` (shared).
  static const DoubleToken doubleTabsPanelGap = DoubleToken('hero.double.tabs.panelGap');

  /// `hero.double.tabs.panelPadding` (shared).
  static const DoubleToken doubleTabsPanelPadding = DoubleToken('hero.double.tabs.panelPadding');

  /// `hero.double.tabs.tabHeight` (shared).
  static const DoubleToken doubleTabsTabHeight = DoubleToken('hero.double.tabs.tabHeight');

  /// `hero.double.tabs.tabPaddingX` (shared).
  static const DoubleToken doubleTabsTabPaddingX = DoubleToken('hero.double.tabs.tabPaddingX');

  /// `hero.double.tooltip.fontSize` (shared).
  static const DoubleToken doubleTooltipFontSize = DoubleToken('hero.double.tooltip.fontSize');

  /// `hero.double.tooltip.maxWidth` (shared).
  static const DoubleToken doubleTooltipMaxWidth = DoubleToken('hero.double.tooltip.maxWidth');

  /// `hero.double.tooltip.padding` (shared).
  static const DoubleToken doubleTooltipPadding = DoubleToken('hero.double.tooltip.padding');

  /// `hero.duration.skeleton` (shared).
  static const DurationToken durationSkeleton = DurationToken('hero.duration.skeleton');

  /// `hero.duration.spin` (shared).
  static const DurationToken durationSpin = DurationToken('hero.duration.spin');

  /// `hero.duration.tooltip.closeDelay` (shared).
  static const DurationToken durationTooltipCloseDelay = DurationToken('hero.duration.tooltip.closeDelay');

  /// `hero.duration.tooltip.delay` (shared).
  static const DurationToken durationTooltipDelay = DurationToken('hero.duration.tooltip.delay');

  /// `hero.duration.transition.base` (shared).
  static const DurationToken durationTransitionBase = DurationToken('hero.duration.transition.base');

  /// `hero.duration.transition.fast` (shared).
  static const DurationToken durationTransitionFast = DurationToken('hero.duration.transition.fast');

  /// `hero.duration.transition.medium` (shared).
  static const DurationToken durationTransitionMedium = DurationToken('hero.duration.transition.medium');

  /// `hero.duration.transition.slow` (shared).
  static const DurationToken durationTransitionSlow = DurationToken('hero.duration.transition.slow');

  /// `hero.weight.medium` (shared).
  static const FontWeightToken weightMedium = FontWeightToken('hero.weight.medium');

  /// `hero.weight.semibold` (shared).
  static const FontWeightToken weightSemibold = FontWeightToken('hero.weight.semibold');

  /// `hero.radius.2xl` (shared).
  static const RadiusToken radius2xl = RadiusToken('hero.radius.2xl');

  /// `hero.radius.3xl` (shared).
  static const RadiusToken radius3xl = RadiusToken('hero.radius.3xl');

  /// `hero.radius.4xl` (shared).
  static const RadiusToken radius4xl = RadiusToken('hero.radius.4xl');

  /// `hero.radius.avatar.radius.lg` (shared).
  static const RadiusToken radiusAvatarRadiusLg = RadiusToken('hero.radius.avatar.radius.lg');

  /// `hero.radius.avatar.radius.md` (shared).
  static const RadiusToken radiusAvatarRadiusMd = RadiusToken('hero.radius.avatar.radius.md');

  /// `hero.radius.avatar.radius.sm` (shared).
  static const RadiusToken radiusAvatarRadiusSm = RadiusToken('hero.radius.avatar.radius.sm');

  /// `hero.radius.badge.radius.lg` (shared).
  static const RadiusToken radiusBadgeRadiusLg = RadiusToken('hero.radius.badge.radius.lg');

  /// `hero.radius.badge.radius.md` (shared).
  static const RadiusToken radiusBadgeRadiusMd = RadiusToken('hero.radius.badge.radius.md');

  /// `hero.radius.badge.radius.sm` (shared).
  static const RadiusToken radiusBadgeRadiusSm = RadiusToken('hero.radius.badge.radius.sm');

  /// `hero.radius.button.radius` (shared).
  static const RadiusToken radiusButtonRadius = RadiusToken('hero.radius.button.radius');

  /// `hero.radius.card.radius` (shared).
  static const RadiusToken radiusCardRadius = RadiusToken('hero.radius.card.radius');

  /// `hero.radius.checkbox.radius` (shared).
  static const RadiusToken radiusCheckboxRadius = RadiusToken('hero.radius.checkbox.radius');

  /// `hero.radius.chip.radius` (shared).
  static const RadiusToken radiusChipRadius = RadiusToken('hero.radius.chip.radius');

  /// `hero.radius.field` (shared).
  static const RadiusToken radiusField = RadiusToken('hero.radius.field');

  /// `hero.radius.input.radius` (shared).
  static const RadiusToken radiusInputRadius = RadiusToken('hero.radius.input.radius');

  /// `hero.radius.lg` (shared).
  static const RadiusToken radiusLg = RadiusToken('hero.radius.lg');

  /// `hero.radius.md` (shared).
  static const RadiusToken radiusMd = RadiusToken('hero.radius.md');

  /// `hero.radius.modal.radius` (shared).
  static const RadiusToken radiusModalRadius = RadiusToken('hero.radius.modal.radius');

  /// `hero.radius.progress.radius` (shared).
  static const RadiusToken radiusProgressRadius = RadiusToken('hero.radius.progress.radius');

  /// `hero.radius.radio.radius` (shared).
  static const RadiusToken radiusRadioRadius = RadiusToken('hero.radius.radio.radius');

  /// `hero.radius.skeleton.radius` (shared).
  static const RadiusToken radiusSkeletonRadius = RadiusToken('hero.radius.skeleton.radius');

  /// `hero.radius.sm` (shared).
  static const RadiusToken radiusSm = RadiusToken('hero.radius.sm');

  /// `hero.radius.switch.controlRadius.lg` (shared).
  static const RadiusToken radiusSwitchControlRadiusLg = RadiusToken('hero.radius.switch.controlRadius.lg');

  /// `hero.radius.switch.controlRadius.md` (shared).
  static const RadiusToken radiusSwitchControlRadiusMd = RadiusToken('hero.radius.switch.controlRadius.md');

  /// `hero.radius.switch.controlRadius.sm` (shared).
  static const RadiusToken radiusSwitchControlRadiusSm = RadiusToken('hero.radius.switch.controlRadius.sm');

  /// `hero.radius.switch.thumbRadius.lg` (shared).
  static const RadiusToken radiusSwitchThumbRadiusLg = RadiusToken('hero.radius.switch.thumbRadius.lg');

  /// `hero.radius.switch.thumbRadius.md` (shared).
  static const RadiusToken radiusSwitchThumbRadiusMd = RadiusToken('hero.radius.switch.thumbRadius.md');

  /// `hero.radius.switch.thumbRadius.sm` (shared).
  static const RadiusToken radiusSwitchThumbRadiusSm = RadiusToken('hero.radius.switch.thumbRadius.sm');

  /// `hero.radius.tabs.containerRadius` (shared).
  static const RadiusToken radiusTabsContainerRadius = RadiusToken('hero.radius.tabs.containerRadius');

  /// `hero.radius.tabs.tabRadius` (shared).
  static const RadiusToken radiusTabsTabRadius = RadiusToken('hero.radius.tabs.tabRadius');

  /// `hero.radius.tooltip.radius` (shared).
  static const RadiusToken radiusTooltipRadius = RadiusToken('hero.radius.tooltip.radius');

  /// `hero.radius.xl` (shared).
  static const RadiusToken radiusXl = RadiusToken('hero.radius.xl');

  /// `hero.radius.xs` (shared).
  static const RadiusToken radiusXs = RadiusToken('hero.radius.xs');

  /// `hero.space.0.5` (shared).
  static const SpaceToken space05 = SpaceToken('hero.space.0.5');

  /// `hero.space.1` (shared).
  static const SpaceToken space1 = SpaceToken('hero.space.1');

  /// `hero.space.1.5` (shared).
  static const SpaceToken space15 = SpaceToken('hero.space.1.5');

  /// `hero.space.10` (shared).
  static const SpaceToken space10 = SpaceToken('hero.space.10');

  /// `hero.space.2` (shared).
  static const SpaceToken space2 = SpaceToken('hero.space.2');

  /// `hero.space.3` (shared).
  static const SpaceToken space3 = SpaceToken('hero.space.3');

  /// `hero.space.4` (shared).
  static const SpaceToken space4 = SpaceToken('hero.space.4');

  /// `hero.space.5` (shared).
  static const SpaceToken space5 = SpaceToken('hero.space.5');

  /// `hero.space.6` (shared).
  static const SpaceToken space6 = SpaceToken('hero.space.6');

  /// `hero.space.8` (shared).
  static const SpaceToken space8 = SpaceToken('hero.space.8');

  /// `hero.type.2xl` (shared).
  static const TextStyleToken type2xl = TextStyleToken('hero.type.2xl');

  /// `hero.type.3xl` (shared).
  static const TextStyleToken type3xl = TextStyleToken('hero.type.3xl');

  /// `hero.type.4xl` (shared).
  static const TextStyleToken type4xl = TextStyleToken('hero.type.4xl');

  /// `hero.type.base` (shared).
  static const TextStyleToken typeBase = TextStyleToken('hero.type.base');

  /// `hero.type.lg` (shared).
  static const TextStyleToken typeLg = TextStyleToken('hero.type.lg');

  /// `hero.type.sm` (shared).
  static const TextStyleToken typeSm = TextStyleToken('hero.type.sm');

  /// `hero.type.xl` (shared).
  static const TextStyleToken typeXl = TextStyleToken('hero.type.xl');

  /// `hero.type.xs` (shared).
  static const TextStyleToken typeXs = TextStyleToken('hero.type.xs');

}

/// Resolved `color` token values.
final Map<ColorToken, Color> heroRoleColorsLight = {
  HeroTokens.colorAccent: const Color(0xFF0485F7),
  HeroTokens.colorAccentForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorAccentHover: const Color(0xFF3592F9),
  HeroTokens.colorAccentSoft: const Color(0x260485F7),
  HeroTokens.colorAccentSoftForeground: const Color(0xFF1E63AE),
  HeroTokens.colorAccentSoftHover: const Color(0x330485F7),
  HeroTokens.colorBackdrop: const Color(0x80000000),
  HeroTokens.colorBackground: const Color(0xFFF5F5F5),
  HeroTokens.colorBackgroundInverse: const Color(0xFF18181B),
  HeroTokens.colorBackgroundSecondary: const Color(0xFFEBEBEB),
  HeroTokens.colorBackgroundTertiary: const Color(0xFFE1E1E1),
  HeroTokens.colorBlack: const Color(0xFF000000),
  HeroTokens.colorBorder: const Color(0xFFDEDEE0),
  HeroTokens.colorBorderSecondary: const Color(0xFFC6C6C7),
  HeroTokens.colorBorderTertiary: const Color(0xFFA8A8A9),
  HeroTokens.colorDanger: const Color(0xFFFF383C),
  HeroTokens.colorDangerForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorDangerHover: const Color(0xFFFF5551),
  HeroTokens.colorDangerSoft: const Color(0x26FF383C),
  HeroTokens.colorDangerSoftForeground: const Color(0xFFA43433),
  HeroTokens.colorDangerSoftHover: const Color(0x33FF383C),
  HeroTokens.colorDefault: const Color(0xFFEBEBEC),
  HeroTokens.colorDefaultForeground: const Color(0xFF18181B),
  HeroTokens.colorDefaultHover: const Color(0xFFE1E1E2),
  HeroTokens.colorDefaultSoft: const Color(0x80EBEBEC),
  HeroTokens.colorDefaultSoftForeground: const Color(0xFF18181B),
  HeroTokens.colorDefaultSoftHover: const Color(0x99EBEBEC),
  HeroTokens.colorEclipse: const Color(0xFF18181B),
  HeroTokens.colorField: const Color(0xFFFFFFFF),
  HeroTokens.colorFieldBorder: const Color(0x00000000),
  HeroTokens.colorFieldBorderFocus: const Color(0x3A18181B),
  HeroTokens.colorFieldBorderHover: const Color(0x1A18181B),
  HeroTokens.colorFieldFocus: const Color(0xFFFFFFFF),
  HeroTokens.colorFieldForeground: const Color(0xFF18181B),
  HeroTokens.colorFieldHover: const Color(0xFFF9F9F9),
  HeroTokens.colorFieldPlaceholder: const Color(0xFF71717A),
  HeroTokens.colorFocus: const Color(0xFF0485F7),
  HeroTokens.colorForeground: const Color(0xFF18181B),
  HeroTokens.colorLink: const Color(0xFF18181B),
  HeroTokens.colorMuted: const Color(0xFF71717A),
  HeroTokens.colorOutlineHover: const Color(0x99EBEBEC),
  HeroTokens.colorOverlay: const Color(0xFFFFFFFF),
  HeroTokens.colorOverlayForeground: const Color(0xFF18181B),
  HeroTokens.colorSegment: const Color(0xFFFFFFFF),
  HeroTokens.colorSegmentForeground: const Color(0xFF18181B),
  HeroTokens.colorSeparator: const Color(0xFFE4E4E7),
  HeroTokens.colorSeparatorSecondary: const Color(0xFFD8D8D8),
  HeroTokens.colorSeparatorTertiary: const Color(0xFFCDCDCE),
  HeroTokens.colorSnow: const Color(0xFFFCFCFC),
  HeroTokens.colorSuccess: const Color(0xFF17C964),
  HeroTokens.colorSuccessForeground: const Color(0xFF18181B),
  HeroTokens.colorSuccessHover: const Color(0xFF22B55D),
  HeroTokens.colorSuccessSoft: const Color(0x2617C964),
  HeroTokens.colorSuccessSoftForeground: const Color(0xFF2B7745),
  HeroTokens.colorSuccessSoftHover: const Color(0x3317C964),
  HeroTokens.colorSurface: const Color(0xFFFFFFFF),
  HeroTokens.colorSurfaceForeground: const Color(0xFF18181B),
  HeroTokens.colorSurfaceHover: const Color(0xFFEAEAEA),
  HeroTokens.colorSurfaceSecondary: const Color(0xFFEFEFF0),
  HeroTokens.colorSurfaceSecondaryForeground: const Color(0xFF18181B),
  HeroTokens.colorSurfaceTertiary: const Color(0xFFEAEAEB),
  HeroTokens.colorSurfaceTertiaryForeground: const Color(0xFF18181B),
  HeroTokens.colorTransparent: const Color(0x00000000),
  HeroTokens.colorWarning: const Color(0xFFF5A524),
  HeroTokens.colorWarningForeground: const Color(0xFF18181B),
  HeroTokens.colorWarningHover: const Color(0xFFDC952A),
  HeroTokens.colorWarningSoft: const Color(0x26F5A524),
  HeroTokens.colorWarningSoftForeground: const Color(0xFF855F2E),
  HeroTokens.colorWarningSoftHover: const Color(0x33F5A524),
  HeroTokens.colorWhite: const Color(0xFFFFFFFF),
};

/// Resolved `color` token values.
final Map<ColorToken, Color> heroRoleColorsDark = {
  HeroTokens.colorAccent: const Color(0xFF0485F7),
  HeroTokens.colorAccentForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorAccentHover: const Color(0xFF3592F9),
  HeroTokens.colorAccentSoft: const Color(0x1F0485F7),
  HeroTokens.colorAccentSoftForeground: const Color(0xFF61A8FB),
  HeroTokens.colorAccentSoftHover: const Color(0x290485F7),
  HeroTokens.colorBackdrop: const Color(0x99000000),
  HeroTokens.colorBackground: const Color(0xFF060607),
  HeroTokens.colorBackgroundInverse: const Color(0xFFFCFCFC),
  HeroTokens.colorBackgroundSecondary: const Color(0xFF0D0D0E),
  HeroTokens.colorBackgroundTertiary: const Color(0xFF141415),
  HeroTokens.colorBlack: const Color(0xFF000000),
  HeroTokens.colorBorder: const Color(0xFF28282C),
  HeroTokens.colorBorderSecondary: const Color(0xFF434345),
  HeroTokens.colorBorderTertiary: const Color(0xFF5C5C5E),
  HeroTokens.colorDanger: const Color(0xFFDB3B3E),
  HeroTokens.colorDangerForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorDangerHover: const Color(0xFFE15451),
  HeroTokens.colorDangerSoft: const Color(0x26DB3B3E),
  HeroTokens.colorDangerSoftForeground: const Color(0xFFEB7872),
  HeroTokens.colorDangerSoftHover: const Color(0x33DB3B3E),
  HeroTokens.colorDefault: const Color(0xFF27272A),
  HeroTokens.colorDefaultForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorDefaultHover: const Color(0xFF2E2E31),
  HeroTokens.colorDefaultSoft: const Color(0x8027272A),
  HeroTokens.colorDefaultSoftForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorDefaultSoftHover: const Color(0x9927272A),
  HeroTokens.colorEclipse: const Color(0xFF18181B),
  HeroTokens.colorField: const Color(0xFF18181B),
  HeroTokens.colorFieldBorder: const Color(0x00000000),
  HeroTokens.colorFieldBorderFocus: const Color(0x3AFCFCFC),
  HeroTokens.colorFieldBorderHover: const Color(0x1AFCFCFC),
  HeroTokens.colorFieldFocus: const Color(0xFF18181B),
  HeroTokens.colorFieldForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorFieldHover: const Color(0xFF1C1C1F),
  HeroTokens.colorFieldPlaceholder: const Color(0xFF9F9FA9),
  HeroTokens.colorFocus: const Color(0xFF0485F7),
  HeroTokens.colorForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorLink: const Color(0xFFFCFCFC),
  HeroTokens.colorMuted: const Color(0xFF9F9FA9),
  HeroTokens.colorOutlineHover: const Color(0x99EBEBEC),
  HeroTokens.colorOverlay: const Color(0xFF18181B),
  HeroTokens.colorOverlayForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorSegment: const Color(0xFF46464C),
  HeroTokens.colorSegmentForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorSeparator: const Color(0xFF212124),
  HeroTokens.colorSeparatorSecondary: const Color(0xFF343437),
  HeroTokens.colorSeparatorTertiary: const Color(0xFF3C3C3F),
  HeroTokens.colorSnow: const Color(0xFFFCFCFC),
  HeroTokens.colorSuccess: const Color(0xFF17C964),
  HeroTokens.colorSuccessForeground: const Color(0xFF18181B),
  HeroTokens.colorSuccessHover: const Color(0xFF22B55D),
  HeroTokens.colorSuccessSoft: const Color(0x1F17C964),
  HeroTokens.colorSuccessSoftForeground: const Color(0xFF74D88F),
  HeroTokens.colorSuccessSoftHover: const Color(0x2917C964),
  HeroTokens.colorSurface: const Color(0xFF18181B),
  HeroTokens.colorSurfaceForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorSurfaceHover: const Color(0xFF27272A),
  HeroTokens.colorSurfaceSecondary: const Color(0xFF232325),
  HeroTokens.colorSurfaceSecondaryForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorSurfaceTertiary: const Color(0xFF262728),
  HeroTokens.colorSurfaceTertiaryForeground: const Color(0xFFFCFCFC),
  HeroTokens.colorTransparent: const Color(0x00000000),
  HeroTokens.colorWarning: const Color(0xFFF7B750),
  HeroTokens.colorWarningForeground: const Color(0xFF18181B),
  HeroTokens.colorWarningHover: const Color(0xFFDEA54C),
  HeroTokens.colorWarningSoft: const Color(0x1FF7B750),
  HeroTokens.colorWarningSoftForeground: const Color(0xFFF9CB86),
  HeroTokens.colorWarningSoftHover: const Color(0x29F7B750),
  HeroTokens.colorWhite: const Color(0xFFFFFFFF),
};

/// Resolved `radius` token values.
final Map<RadiusToken, Radius> heroRadiusValues = {
  HeroTokens.radius2xl: const Radius.circular(16.0),
  HeroTokens.radius3xl: const Radius.circular(24.0),
  HeroTokens.radius4xl: const Radius.circular(32.0),
  HeroTokens.radiusAvatarRadiusLg: const Radius.circular(24.0),
  HeroTokens.radiusAvatarRadiusMd: const Radius.circular(24.0),
  HeroTokens.radiusAvatarRadiusSm: const Radius.circular(16.0),
  HeroTokens.radiusBadgeRadiusLg: const Radius.circular(16.0),
  HeroTokens.radiusBadgeRadiusMd: const Radius.circular(24.0),
  HeroTokens.radiusBadgeRadiusSm: const Radius.circular(12.0),
  HeroTokens.radiusButtonRadius: const Radius.circular(24.0),
  HeroTokens.radiusCardRadius: const Radius.circular(24.0),
  HeroTokens.radiusCheckboxRadius: const Radius.circular(6.0),
  HeroTokens.radiusChipRadius: const Radius.circular(16.0),
  HeroTokens.radiusField: const Radius.circular(12.0),
  HeroTokens.radiusInputRadius: const Radius.circular(12.0),
  HeroTokens.radiusLg: const Radius.circular(8.0),
  HeroTokens.radiusMd: const Radius.circular(6.0),
  HeroTokens.radiusModalRadius: const Radius.circular(24.0),
  HeroTokens.radiusProgressRadius: const Radius.circular(4.0),
  HeroTokens.radiusRadioRadius: const Radius.circular(8.0),
  HeroTokens.radiusSkeletonRadius: const Radius.circular(4.0),
  HeroTokens.radiusSm: const Radius.circular(4.0),
  HeroTokens.radiusSwitchControlRadiusLg: const Radius.circular(12.0),
  HeroTokens.radiusSwitchControlRadiusMd: const Radius.circular(12.0),
  HeroTokens.radiusSwitchControlRadiusSm: const Radius.circular(8.0),
  HeroTokens.radiusSwitchThumbRadiusLg: const Radius.circular(12.0),
  HeroTokens.radiusSwitchThumbRadiusMd: const Radius.circular(8.0),
  HeroTokens.radiusSwitchThumbRadiusSm: const Radius.circular(6.0),
  HeroTokens.radiusTabsContainerRadius: const Radius.circular(20.0),
  HeroTokens.radiusTabsTabRadius: const Radius.circular(24.0),
  HeroTokens.radiusTooltipRadius: const Radius.circular(12.0),
  HeroTokens.radiusXl: const Radius.circular(12.0),
  HeroTokens.radiusXs: const Radius.circular(2.0),
};

/// Resolved `space` token values.
final Map<SpaceToken, double> heroSpaceValues = {
  HeroTokens.space05: 2.0,
  HeroTokens.space1: 4.0,
  HeroTokens.space15: 6.0,
  HeroTokens.space10: 40.0,
  HeroTokens.space2: 8.0,
  HeroTokens.space3: 12.0,
  HeroTokens.space4: 16.0,
  HeroTokens.space5: 20.0,
  HeroTokens.space6: 24.0,
  HeroTokens.space8: 32.0,
};

/// Resolved `double` token values.
final Map<DoubleToken, double> heroDoubleValues = {
  HeroTokens.doubleAvatarFallbackFontSizeLg: 16.0,
  HeroTokens.doubleAvatarFallbackFontSizeMd: 14.0,
  HeroTokens.doubleAvatarFallbackFontSizeSm: 14.0,
  HeroTokens.doubleAvatarSizeLg: 48.0,
  HeroTokens.doubleAvatarSizeMd: 40.0,
  HeroTokens.doubleAvatarSizeSm: 32.0,
  HeroTokens.doubleBadgeBorderWidth: 1.0,
  HeroTokens.doubleBadgeFontSizeLg: 14.0,
  HeroTokens.doubleBadgeFontSizeMd: 12.0,
  HeroTokens.doubleBadgeFontSizeSm: 10.0,
  HeroTokens.doubleBadgeGap: 2.0,
  HeroTokens.doubleBadgeMinSizeLg: 32.0,
  HeroTokens.doubleBadgeMinSizeMd: 28.0,
  HeroTokens.doubleBadgeMinSizeSm: 16.0,
  HeroTokens.doubleBadgePaddingXLg: 10.0,
  HeroTokens.doubleBadgePaddingXMd: 8.0,
  HeroTokens.doubleBadgePaddingXSm: 6.0,
  HeroTokens.doubleBadgePaddingYLg: 4.0,
  HeroTokens.doubleBadgePaddingYMd: 3.0,
  HeroTokens.doubleBadgePaddingYSm: 2.0,
  HeroTokens.doubleBorderWidth: 1.0,
  HeroTokens.doubleButtonFocusRingOffset: 2.0,
  HeroTokens.doubleButtonFocusRingWidth: 2.0,
  HeroTokens.doubleButtonFontSizeLg: 16.0,
  HeroTokens.doubleButtonFontSizeMd: 14.0,
  HeroTokens.doubleButtonFontSizeSm: 14.0,
  HeroTokens.doubleButtonGap: 8.0,
  HeroTokens.doubleButtonHeightLg: 40.0,
  HeroTokens.doubleButtonHeightMd: 36.0,
  HeroTokens.doubleButtonHeightSm: 32.0,
  HeroTokens.doubleButtonIconOnlyWidthLg: 40.0,
  HeroTokens.doubleButtonIconOnlyWidthMd: 36.0,
  HeroTokens.doubleButtonIconOnlyWidthSm: 32.0,
  HeroTokens.doubleButtonIconSizeLg: 20.0,
  HeroTokens.doubleButtonIconSizeMd: 20.0,
  HeroTokens.doubleButtonIconSizeSm: 16.0,
  HeroTokens.doubleButtonPaddingXLg: 16.0,
  HeroTokens.doubleButtonPaddingXMd: 16.0,
  HeroTokens.doubleButtonPaddingXSm: 12.0,
  HeroTokens.doubleButtonPressScaleLg: 0.96,
  HeroTokens.doubleButtonPressScaleMd: 0.97,
  HeroTokens.doubleButtonPressScaleSm: 0.98,
  HeroTokens.doubleButtonTransitionBackgroundMs: 100.0,
  HeroTokens.doubleButtonTransitionTransformMs: 250.0,
  HeroTokens.doubleCardDescriptionFontSize: 14.0,
  HeroTokens.doubleCardDescriptionLineHeight: 20.0,
  HeroTokens.doubleCardGap: 12.0,
  HeroTokens.doubleCardPadding: 16.0,
  HeroTokens.doubleCardTitleFontSize: 14.0,
  HeroTokens.doubleCardTitleLineHeight: 24.0,
  HeroTokens.doubleCheckboxIconSize: 10.0,
  HeroTokens.doubleCheckboxSize: 16.0,
  HeroTokens.doubleChipFontSizeLg: 14.0,
  HeroTokens.doubleChipFontSizeMd: 12.0,
  HeroTokens.doubleChipFontSizeSm: 12.0,
  HeroTokens.doubleChipGap: 2.0,
  HeroTokens.doubleChipPaddingXLg: 12.0,
  HeroTokens.doubleChipPaddingXMd: 8.0,
  HeroTokens.doubleChipPaddingXSm: 4.0,
  HeroTokens.doubleChipPaddingYLg: 4.0,
  HeroTokens.doubleChipPaddingYMd: 2.0,
  HeroTokens.doubleChipPaddingYSm: 0.0,
  HeroTokens.doubleDisabledOpacity: 0.5,
  HeroTokens.doubleFocusRingWidth: 2.0,
  HeroTokens.doubleInputFontSize: 14.0,
  HeroTokens.doubleInputMinHeight: 36.0,
  HeroTokens.doubleInputPaddingX: 12.0,
  HeroTokens.doubleInputPaddingY: 8.0,
  HeroTokens.doubleInputTransitionMs: 150.0,
  HeroTokens.doubleModalMaxWidthLg: 512.0,
  HeroTokens.doubleModalMaxWidthMd: 448.0,
  HeroTokens.doubleModalMaxWidthSm: 384.0,
  HeroTokens.doubleModalMaxWidthXs: 320.0,
  HeroTokens.doubleModalPadding: 24.0,
  HeroTokens.doubleProgressFontSize: 14.0,
  HeroTokens.doubleProgressTrackHeight: 8.0,
  HeroTokens.doubleProgressTransitionMs: 300.0,
  HeroTokens.doubleRadioIndicatorSize: 6.0,
  HeroTokens.doubleRadioSize: 16.0,
  HeroTokens.doubleRingOffset: 2.0,
  HeroTokens.doubleSeparatorThickness: 1.0,
  HeroTokens.doubleSkeletonAnimationMs: 2000.0,
  HeroTokens.doubleSkeletonOpacity: 0.7,
  HeroTokens.doubleSpinnerAnimationMs: 750.0,
  HeroTokens.doubleSpinnerSizeLg: 32.0,
  HeroTokens.doubleSpinnerSizeMd: 24.0,
  HeroTokens.doubleSpinnerSizeSm: 16.0,
  HeroTokens.doubleSpinnerSizeXl: 40.0,
  HeroTokens.doubleSpinnerStrokeWidth: 3.0,
  HeroTokens.doubleSwitchControlHeightLg: 24.0,
  HeroTokens.doubleSwitchControlHeightMd: 20.0,
  HeroTokens.doubleSwitchControlHeightSm: 16.0,
  HeroTokens.doubleSwitchControlWidthLg: 48.0,
  HeroTokens.doubleSwitchControlWidthMd: 40.0,
  HeroTokens.doubleSwitchControlWidthSm: 32.0,
  HeroTokens.doubleSwitchThumbHeightLg: 20.0,
  HeroTokens.doubleSwitchThumbHeightMd: 16.0,
  HeroTokens.doubleSwitchThumbHeightSm: 12.0,
  HeroTokens.doubleSwitchThumbWidthLg: 27.5,
  HeroTokens.doubleSwitchThumbWidthMd: 22.0,
  HeroTokens.doubleSwitchThumbWidthSm: 16.5,
  HeroTokens.doubleTabsFontSize: 14.0,
  HeroTokens.doubleTabsIndicatorHeight: 2.0,
  HeroTokens.doubleTabsListPadding: 4.0,
  HeroTokens.doubleTabsPanelGap: 16.0,
  HeroTokens.doubleTabsPanelPadding: 8.0,
  HeroTokens.doubleTabsTabHeight: 32.0,
  HeroTokens.doubleTabsTabPaddingX: 16.0,
  HeroTokens.doubleTooltipFontSize: 12.0,
  HeroTokens.doubleTooltipMaxWidth: 320.0,
  HeroTokens.doubleTooltipPadding: 8.0,
};

/// Resolved `duration` token values.
final Map<DurationToken, Duration> heroDurationValues = {
  HeroTokens.durationSkeleton: const Duration(milliseconds: 2000),
  HeroTokens.durationSpin: const Duration(milliseconds: 750),
  HeroTokens.durationTooltipCloseDelay: const Duration(milliseconds: 500),
  HeroTokens.durationTooltipDelay: const Duration(milliseconds: 1500),
  HeroTokens.durationTransitionBase: const Duration(milliseconds: 150),
  HeroTokens.durationTransitionFast: const Duration(milliseconds: 100),
  HeroTokens.durationTransitionMedium: const Duration(milliseconds: 250),
  HeroTokens.durationTransitionSlow: const Duration(milliseconds: 300),
};

/// Resolved `fontweight` token values.
final Map<FontWeightToken, FontWeight> heroFontWeightValues = {
  HeroTokens.weightMedium: FontWeight.w500,
  HeroTokens.weightSemibold: FontWeight.w600,
};

/// Resolved `textstyle` token values.
final Map<TextStyleToken, TextStyle> heroTextStyleValues = {
  HeroTokens.type2xl: const TextStyle(fontSize: 24.0, height: 1.3333, fontFamily: 'Inter'),
  HeroTokens.type3xl: const TextStyle(fontSize: 30.0, height: 1.2, fontFamily: 'Inter'),
  HeroTokens.type4xl: const TextStyle(fontSize: 36.0, height: 1.1111, fontFamily: 'Inter'),
  HeroTokens.typeBase: const TextStyle(fontSize: 16.0, height: 1.5, fontFamily: 'Inter'),
  HeroTokens.typeLg: const TextStyle(fontSize: 18.0, height: 1.5556, fontFamily: 'Inter'),
  HeroTokens.typeSm: const TextStyle(fontSize: 14.0, height: 1.4286, fontFamily: 'Inter'),
  HeroTokens.typeXl: const TextStyle(fontSize: 20.0, height: 1.4, fontFamily: 'Inter'),
  HeroTokens.typeXs: const TextStyle(fontSize: 12.0, height: 1.3333, fontFamily: 'Inter'),
};

/// Resolved box-shadow token values (light).
final Map<BoxShadowToken, List<BoxShadow>> heroShadowValuesLight = {
  HeroTokens.shadowField: [
      BoxShadow(color: Color(0x0A000000), offset: Offset(0.0, 2.0), blurRadius: 4.0, spreadRadius: 0.0),
      BoxShadow(color: Color(0x0F000000), offset: Offset(0.0, 1.0), blurRadius: 2.0, spreadRadius: 0.0),
      BoxShadow(color: Color(0x0F000000), offset: Offset(0.0, 0.0), blurRadius: 1.0, spreadRadius: 0.0),
    ],
  HeroTokens.shadowOverlay: [
      BoxShadow(color: Color(0x0F000000), offset: Offset(0.0, 2.0), blurRadius: 8.0, spreadRadius: 0.0),
      BoxShadow(color: Color(0x08000000), offset: Offset(0.0, -6.0), blurRadius: 12.0, spreadRadius: 0.0),
      BoxShadow(color: Color(0x14000000), offset: Offset(0.0, 14.0), blurRadius: 28.0, spreadRadius: 0.0),
    ],
  HeroTokens.shadowSurface: [
      BoxShadow(color: Color(0x0A000000), offset: Offset(0.0, 2.0), blurRadius: 4.0, spreadRadius: 0.0),
      BoxShadow(color: Color(0x0F000000), offset: Offset(0.0, 1.0), blurRadius: 2.0, spreadRadius: 0.0),
      BoxShadow(color: Color(0x0F000000), offset: Offset(0.0, 0.0), blurRadius: 1.0, spreadRadius: 0.0),
    ],
};

/// Resolved box-shadow token values (dark).
final Map<BoxShadowToken, List<BoxShadow>> heroShadowValuesDark = {
  HeroTokens.shadowField: [
      BoxShadow(color: Color(0x00000000), offset: Offset(0.0, 0.0), blurRadius: 0.0, spreadRadius: 0.0),
    ],
  HeroTokens.shadowOverlay: [
      BoxShadow(color: Color(0x4CFFFFFF), offset: Offset(0.0, 0.0), blurRadius: 1.0, spreadRadius: 0.0),
    ],
  HeroTokens.shadowSurface: [
      BoxShadow(color: Color(0x00000000), offset: Offset(0.0, 0.0), blurRadius: 0.0, spreadRadius: 0.0),
    ],
};

/// HeroUI v3 easing curves (shared-theme.css --ease-*).
const Cubic heroEaseInOut = Cubic(0.645, 0.045, 0.355, 1.0);
const Cubic heroEaseLinear = Cubic(0.0, 0.0, 1.0, 1.0);
const Cubic heroEaseOut = Cubic(0.215, 0.61, 0.355, 1.0);
const Cubic heroEaseOutFluid = Cubic(0.32, 0.72, 0.0, 1.0);
const Cubic heroEaseSmooth = Cubic(0.25, 0.1, 0.25, 1.0);

/// HeroUI v3 durations in milliseconds for non-styler contexts
/// (styler chains should use the `HeroTokens.duration*` tokens instead).
const int heroSkeletonMs = 2000;
const int heroSpinMs = 750;
const int heroTooltipCloseDelayMs = 500;
const int heroTooltipDelayMs = 1500;
const int heroTransitionBaseMs = 150;
const int heroTransitionFastMs = 100;
const int heroTransitionMediumMs = 250;
const int heroTransitionSlowMs = 300;
const int heroButtonTransitionBackgroundMs = 100;
const int heroButtonTransitionTransformMs = 250;
const int heroInputTransitionMs = 150;
const int heroProgressTransitionMs = 300;
const int heroSkeletonAnimationMs = 2000;
const int heroSpinnerAnimationMs = 750;
const double heroButtonPressScaleLg = 0.96;
const double heroButtonPressScaleMd = 0.97;
const double heroButtonPressScaleSm = 0.98;

/// Pinned source metadata for the generated tokens — what token tests
/// assert against (version, file hashes, inventory counts).
abstract final class HeroSourceManifest {
  const HeroSourceManifest._();
  static const String name = '@heroui/styles';
  static const String version = '3.2.4';
  static const String registryUrl = 'https://registry.npmjs.org/@heroui/styles';
  static const String retrievedAt = '2026-03-10';
  static const Map<String, String> sourceFiles = {
    'heroui-avatar.css': '9114a5d88543a05be090fdd5b97f4940bced0b9156c885e816569866a89b6836',
    'heroui-badge.css': 'e8a2ef1ef462632ceadd3601bde1a08a2cf61f6959c545731f34c67cded19c48',
    'heroui-button.css': '2c57787104d89e9f89a9d2e595c075709b15f54c37af196db5ab4c2a6fec2652',
    'heroui-card.css': 'da156ad4192d4dd969a0a41ea0ee3be5474829113c4dfbbe6527d4fbeda203d8',
    'heroui-checkbox.css': 'ffd9993c33d4b90a6e5feea31a5ed78c93ced9dc2c5551c6755553691a220a4d',
    'heroui-chip.css': 'db2096d96446c7f5945062efca2b3665d81b728954a29120ec7f26f23f19cc36',
    'heroui-input.css': '50348bef7c9e2e698de1a9a7de8d559a270f80228306a6f5de297bbe4df3cf2f',
    'heroui-modal.css': 'b99e489a93658dfb94c69b834c480e5112c77370807aa29d9c67963a2325444f',
    'heroui-progress-bar.css': '65f6d464638795e9c9e9b2a1d8cb50adaad6d4c996b92155da4f53cc29b1e1b4',
    'heroui-radio.css': '1feeb2d7009c40831ee855c54b38d0f37b2206fb0cf8433c5c77882e9cb7ec52',
    'heroui-separator.css': '793d63e15f2448bcd42d4f8f25139af0f9edefb29766b91ace39f7393eeef685',
    'heroui-skeleton.css': '893520dac75245372a1568b1cea34c114c0a8dcf6d497b70e3fc515a7ea3e076',
    'heroui-spinner.css': 'ef0cae38cc86a52672fc1350ad64b8b05f4f3a93489407f621806104b04d5b85',
    'heroui-switch.css': '4a9f78eefb534545ac247031c7bf0643c3c788e889d6b9d2aaac9be64af134dd',
    'heroui-tabs.css': 'bcc3308b29c116b989d488870b0fc391429f76828980d883fe71b25113f7d00c',
    'heroui-tooltip.css': 'b453068c40ebee577c5069a7e11df29d18d94b9600ad19ee053821a5f9c96182',
    'heroui-utilities.css': 'b453068c40ebee577c5069a7e11df29d18d94b9600ad19ee053821a5f9c96182',
  };
  // Inventory counts (generated from the normalized snapshot).
  static const int boxshadowTokenCount = 3;
  static const int colorTokenCount = 70;
  static const int doubleTokenCount = 111;
  static const int durationTokenCount = 8;
  static const int fontweightTokenCount = 2;
  static const int radiusTokenCount = 33;
  static const int spaceTokenCount = 10;
  static const int textstyleTokenCount = 8;
  static const int totalTokenCount = 245;
  static const String fontFamily = 'Inter';
}
