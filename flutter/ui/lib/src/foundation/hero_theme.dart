import 'package:flutter/widgets.dart';
import 'package:mix/mix.dart';

import '../tokens/hero_tokens.dart';

/// HeroUI v3 theme selector.
///
/// The token surface is a small set of semantic roles (accent, default,
/// success, warning, danger, surface, overlay, …) with per-theme values —
/// the same model HeroUI v3 uses (semantic intent, not numbered scales).
enum HeroTheme {
  light(Brightness.light),
  dark(Brightness.dark);

  const HeroTheme(this.brightness);

  /// The [Brightness] this theme renders as.
  final Brightness brightness;

  /// Maps a [Brightness] to the matching [HeroTheme].
  static HeroTheme of(Brightness brightness) =>
      brightness == Brightness.dark ? HeroTheme.dark : HeroTheme.light;
}

/// Per-theme base token map, built once and cached.
///
/// Mix tokens override `==`, so the maps can never be `const`; returning the
/// identical cached instance lets `MixScope` dependents short-circuit on
/// `identical()` instead of deep-comparing hundreds of entries per rebuild.
final Map<HeroTheme, Map<MixToken, Object>> _baseTokenMaps = {};

Map<MixToken, Object> _baseTokenMapFor(HeroTheme theme) {
  return _baseTokenMaps.putIfAbsent(theme, () {
    final isLight = theme == HeroTheme.light;
    return Map.unmodifiable(<MixToken, Object>{
      ...(isLight ? heroRoleColorsLight : heroRoleColorsDark),
      ...heroRadiusValues,
      ...heroSpaceValues,
      ...heroDoubleValues,
      ...heroDurationValues,
      ...heroFontWeightValues,
      ...heroTextStyleValues,
      ...(isLight ? heroShadowValuesLight : heroShadowValuesDark),
    });
  });
}

/// Typed overrides for a [HeroScope].
///
/// Has value equality so the scope's `updateShouldNotify` only fires when the
/// overrides actually change.
@immutable
class HeroThemeOverrides {
  const HeroThemeOverrides({this.colors = const {}});

  /// Overrides color tokens by token name. Keys must match a `HeroTokens`
  /// color handle (e.g. `HeroTokens.colorAccent.name`).
  final Map<String, Color> colors;

  bool get isEmpty => colors.isEmpty;

  HeroThemeOverrides copyWith({Map<String, Color>? colors}) =>
      HeroThemeOverrides(colors: colors ?? this.colors);

  @override
  bool operator ==(Object other) {
    if (identical(this, other)) return true;
    if (other is! HeroThemeOverrides) return false;
    if (colors.length != other.colors.length) return false;
    for (final entry in colors.entries) {
      if (other.colors[entry.key] != entry.value) return false;
    }
    return true;
  }

  @override
  int get hashCode => Object.hashAll(colors.entries.expand((e) => [e.key, e.value]));
}

/// Builds the token map a [HeroScope] hands to `MixScope`.
///
/// With no overrides the cached per-theme base map is returned as-is
/// (identical instance); overrides produce a fresh shallow copy.
Map<MixToken, Object> buildHeroTokenMap(
  HeroTheme theme, {
  HeroThemeOverrides overrides = const HeroThemeOverrides(),
}) {
  final base = _baseTokenMapFor(theme);
  if (overrides.isEmpty) return base;
  // ColorToken equality is name-based, so a token built from the override key
  // matches the corresponding HeroTokens handle.
  final colors = <ColorToken, Color>{
    for (final entry in overrides.colors.entries) ColorToken(entry.key): entry.value,
  };
  return {...base, ...colors};
}
