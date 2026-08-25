import 'package:flutter/widgets.dart';
import 'package:mix/mix.dart';

import 'hero_theme.dart';

/// Provides HeroUI v3 design tokens to a subtree via a [MixScope].
///
/// **Required** above any `Hero*` widget (and above Remix widgets styled with
/// `hero*Style(...)` recipes): recipes reference tokens that resolve against
/// this scope.
///
/// Place it where the host hands `WidgetsApp` its builder — under a
/// `MaterialApp`, wrap the `builder:`'s child so pushed routes keep the scope:
///
/// ```dart
/// MaterialApp(
///   builder: (context, child) => HeroScope(
///     theme: MediaQuery.platformBrightnessOf(context) == Brightness.dark
///         ? HeroTheme.dark
///         : HeroTheme.light,
///     child: child!,
///   ),
///   home: const MyScreen(),
/// )
/// ```
class HeroScope extends StatelessWidget {
  const HeroScope({
    super.key,
    this.theme,
    this.overrides = const HeroThemeOverrides(),
    this.fontFamily = 'Inter',
    required this.child,
  });

  /// The theme to apply. Defaults to the ambient platform brightness when
  /// null (resolved in [themeOf]).
  final HeroTheme? theme;

  /// Token overrides (value equality; only notifies dependents on change).
  final HeroThemeOverrides overrides;

  /// Default font family applied to ALL text below the scope (bare `Text`
  /// included), matching HeroUI's Inter. Defaults to 'Inter'.
  final String fontFamily;

  final Widget child;

  @override
  Widget build(BuildContext context) {
    final resolved = theme ?? HeroTheme.of(MediaQuery.platformBrightnessOf(context));
    return _HeroScopeInherited(
      theme: resolved,
      overrides: overrides,
      child: MixScope(
        tokens: buildHeroTokenMap(resolved, overrides: overrides),
        child: DefaultTextStyle.merge(
          style: TextStyle(fontFamily: fontFamily),
          child: DefaultTextStyle(
            // Belt-and-braces against the host's ambient text style: any
            // decoration inherited from above the scope (e.g. a link-style
            // underline) must NOT leak into Hero content or overlays. A
            // merge with decoration: none only overrides the decoration —
            // every other field (color, size, …) still inherits.
            style: const TextStyle(decoration: TextDecoration.none),
            child: child,
          ),
        ),
      ),
    );
  }

  /// The nearest [HeroScope]'s theme, or the platform brightness default.
  static HeroTheme themeOf(BuildContext context) =>
      _HeroScopeInherited.maybeOf(context)?.theme ??
      HeroTheme.of(MediaQuery.platformBrightnessOf(context));

  /// The nearest [HeroScope]'s theme, or null when no scope is present.
  static HeroTheme? maybeThemeOf(BuildContext context) =>
      _HeroScopeInherited.maybeOf(context)?.theme;

  /// The nearest [HeroScope]'s overrides (empty when no scope is present).
  static HeroThemeOverrides overridesOf(BuildContext context) =>
      _HeroScopeInherited.maybeOf(context)?.overrides ??
      const HeroThemeOverrides();

  /// Convenience: resolves a color token against the ambient scope.
  static Color colorOf(BuildContext context, ColorToken token) =>
      token.resolve(context);
}

class _HeroScopeInherited extends InheritedWidget {
  const _HeroScopeInherited({
    required this.theme,
    required this.overrides,
    required super.child,
  });

  final HeroTheme theme;
  final HeroThemeOverrides overrides;

  static _HeroScopeInherited? maybeOf(BuildContext context) =>
      context.dependOnInheritedWidgetOfExactType<_HeroScopeInherited>();

  @override
  bool updateShouldNotify(_HeroScopeInherited oldWidget) =>
      theme != oldWidget.theme || overrides != oldWidget.overrides;
}
