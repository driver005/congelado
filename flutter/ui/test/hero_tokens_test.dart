import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

void main() {
  group('HeroSourceManifest', () {
    test('pins the expected upstream source', () {
      expect(HeroSourceManifest.name, '@heroui/styles');
      expect(HeroSourceManifest.version, '3.2.4');
      expect(HeroSourceManifest.registryUrl, contains('registry.npmjs.org'));
      expect(HeroSourceManifest.sourceFiles, isNotEmpty);
    });

    test('inventory counts match the generated maps (no drift)', () {
      expect(heroRoleColorsLight.length, HeroSourceManifest.colorTokenCount);
      expect(heroRoleColorsDark.length, HeroSourceManifest.colorTokenCount);
      expect(heroRadiusValues.length, HeroSourceManifest.radiusTokenCount);
      expect(heroSpaceValues.length, HeroSourceManifest.spaceTokenCount);
      expect(heroDoubleValues.length, HeroSourceManifest.doubleTokenCount);
      expect(heroDurationValues.length, HeroSourceManifest.durationTokenCount);
      expect(heroFontWeightValues.length, HeroSourceManifest.fontweightTokenCount);
      expect(heroTextStyleValues.length, HeroSourceManifest.textstyleTokenCount);
      expect(heroShadowValuesLight.length, HeroSourceManifest.boxshadowTokenCount);
      expect(heroShadowValuesDark.length, HeroSourceManifest.boxshadowTokenCount);
      expect(
        HeroSourceManifest.totalTokenCount,
        HeroSourceManifest.colorTokenCount +
            HeroSourceManifest.radiusTokenCount +
            HeroSourceManifest.spaceTokenCount +
            HeroSourceManifest.doubleTokenCount +
            HeroSourceManifest.durationTokenCount +
            HeroSourceManifest.fontweightTokenCount +
            HeroSourceManifest.textstyleTokenCount +
            HeroSourceManifest.boxshadowTokenCount,
      );
    });
  });

  group('generated token values (pinned @heroui/styles@3.2.4)', () {
    test('light/dark role colors', () {
      // --accent: oklch(0.6204 0.195 253.83) -> #0485F7
      expect(heroRoleColorsLight[HeroTokens.colorAccent], const Color(0xFF0485F7));
      // --background light oklch(0.9702 0 0) / dark oklch(12% 0.005 285.823)
      expect(heroRoleColorsLight[HeroTokens.colorBackground], const Color(0xFFF5F5F5));
      expect(heroRoleColorsDark[HeroTokens.colorBackground], const Color(0xFF060607));
      // dark --foreground is --snow (inherited primitive)
      expect(heroRoleColorsDark[HeroTokens.colorForeground], const Color(0xFFFCFCFC));
      expect(heroRoleColorsLight[HeroTokens.colorForeground], const Color(0xFF18181B));
    });

    test('soft colors are the base color at the source alpha', () {
      // color-mix(in oklab, var(--accent) 15%, transparent) -> accent @ 15%
      expect(
        heroRoleColorsLight[HeroTokens.colorAccentSoft],
        const Color(0x260485F7),
      );
      // color-mix(in oklab, var(--default) 50%, transparent)
      expect(
        heroRoleColorsLight[HeroTokens.colorDefaultSoft],
        const Color(0x80EBEBEC),
      );
    });

    test('radius scale and component radii', () {
      expect(heroRadiusValues[HeroTokens.radiusXs], const Radius.circular(2));
      expect(heroRadiusValues[HeroTokens.radius3xl], const Radius.circular(24));
      expect(heroRadiusValues[HeroTokens.radiusField], const Radius.circular(12));
      expect(heroRadiusValues[HeroTokens.radiusButtonRadius], const Radius.circular(24));
      expect(heroRadiusValues[HeroTokens.radiusInputRadius], const Radius.circular(12));
    });

    test('component metrics', () {
      expect(heroDoubleValues[HeroTokens.doubleButtonHeightMd], 36.0);
      expect(heroDoubleValues[HeroTokens.doubleButtonHeightLg], 40.0);
      expect(heroDoubleValues[HeroTokens.doubleDisabledOpacity], 0.5);
      expect(heroDurationValues[HeroTokens.durationTransitionFast],
          const Duration(milliseconds: 100));
      expect(heroFontWeightValues[HeroTokens.weightMedium], FontWeight.w500);
    });

    test('shadows: light layered, dark none (transparent)', () {
      final light = heroShadowValuesLight[HeroTokens.shadowSurface]!;
      expect(light, hasLength(3));
      expect(light.first.color, const Color(0x0A000000));
      final dark = heroShadowValuesDark[HeroTokens.shadowSurface]!;
      expect(dark, hasLength(1));
      expect(dark.single.color, const Color(0x00000000));
    });
  });

  group('buildHeroTokenMap', () {
    test('returns the identical cached instance for the same theme', () {
      final a = buildHeroTokenMap(HeroTheme.light);
      final b = buildHeroTokenMap(HeroTheme.light);
      expect(identical(a, b), isTrue);
      expect(a[HeroTokens.colorAccent], const Color(0xFF0485F7));
    });

    test('dark map carries dark role values', () {
      final dark = buildHeroTokenMap(HeroTheme.dark);
      expect(dark[HeroTokens.colorForeground], const Color(0xFFFCFCFC));
      expect(dark[HeroTokens.colorSurface], const Color(0xFF18181B));
    });

    test('overrides produce a fresh map with the overridden token', () {
      final over = buildHeroTokenMap(
        HeroTheme.light,
        overrides: const HeroThemeOverrides(
          colors: {'hero.color.accent': Color(0xFFFF0000)},
        ),
      );
      expect(over[HeroTokens.colorAccent], const Color(0xFFFF0000));
      expect(over[HeroTokens.colorBackground],
          buildHeroTokenMap(HeroTheme.light)[HeroTokens.colorBackground]);
    });

    test('empty overrides return the cached base map unchanged', () {
      final base = buildHeroTokenMap(HeroTheme.dark);
      final same = buildHeroTokenMap(HeroTheme.dark, overrides: const HeroThemeOverrides());
      expect(identical(base, same), isTrue);
    });
  });
}
