import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

int _scopeDependentBuilds = 0;

class _CountingDependent extends StatelessWidget {
  const _CountingDependent();

  @override
  Widget build(BuildContext context) {
    // Depend on the inherited scope so the element rebuilds only when the
    // scope notifies (const widget instance -> pump alone is not enough).
    HeroScope.overridesOf(context);
    _scopeDependentBuilds++;
    return const SizedBox();
  }
}

Widget _wrap(Widget child, {HeroTheme? theme}) {
  return HeroScope(
    theme: theme,
    child: MaterialApp(home: child),
  );
}

void main() {
  testWidgets('HeroScope provides tokens to descendants', (tester) async {
    Color? resolved;
    await tester.pumpWidget(
      _wrap(
        Builder(
          builder: (context) {
            resolved = HeroTokens.colorAccent.resolve(context);
            return const SizedBox();
          },
        ),
      ),
    );
    expect(resolved, const Color(0xFF0485F7));
  });

  testWidgets('themeOf falls back to the platform brightness without a scope',
      (tester) async {
    await tester.pumpWidget(const MaterialApp(home: SizedBox()));
    // No HeroScope above; the default must be the platform brightness.
    // (MediaQuery.platformBrightnessOf works inside MaterialApp.)
    final context = tester.element(find.byType(SizedBox));
    expect(
      HeroScope.themeOf(context),
      MediaQuery.platformBrightnessOf(context) == Brightness.dark
          ? HeroTheme.dark
          : HeroTheme.light,
    );
  });

  testWidgets('explicit scope theme wins over platform brightness',
      (tester) async {
    await tester.pumpWidget(
      _wrap(const SizedBox(), theme: HeroTheme.dark),
    );
    final context = tester.element(find.byType(SizedBox));
    expect(HeroScope.themeOf(context), HeroTheme.dark);
    expect(
      HeroTokens.colorForeground.resolve(context),
      const Color(0xFFFCFCFC),
    );
  });

  testWidgets('overridesOf returns the scope overrides', (tester) async {
    const overrides = HeroThemeOverrides(
      colors: {'hero.color.accent': Color(0xFFFF0000)},
    );
    await tester.pumpWidget(
      HeroScope(
        theme: HeroTheme.light,
        overrides: overrides,
        child: const MaterialApp(home: SizedBox()),
      ),
    );
    final context = tester.element(find.byType(SizedBox));
    expect(HeroScope.overridesOf(context), overrides);
    // And the override actually resolves:
    expect(HeroTokens.colorAccent.resolve(context), const Color(0xFFFF0000));
  });

  testWidgets('overridesOf is empty without a scope', (tester) async {
    await tester.pumpWidget(const MaterialApp(home: SizedBox()));
    final context = tester.element(find.byType(SizedBox));
    expect(HeroScope.overridesOf(context).isEmpty, isTrue);
  });

  testWidgets('scope rebuild with equal overrides does not notify dependents',
      (tester) async {
    // The child must be const (same widget instance) so its element only
    // rebuilds when the inherited scope actually notifies — a non-const child
    // would rebuild on every pump because its widget instance changes.
    Widget screen() => const HeroScope(
          theme: HeroTheme.light,
          child: _CountingDependent(),
        );
    await tester.pumpWidget(screen());
    final before = _scopeDependentBuilds;
    await tester.pumpWidget(screen());
    // Same theme + empty overrides -> updateShouldNotify false -> no rebuild.
    expect(_scopeDependentBuilds, before);
  });
}
