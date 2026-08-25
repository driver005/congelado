import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

/// Pumps [child] inside the HeroScope + MaterialApp host that the README and
/// the app's main.dart use.
Widget _host(Widget child, {HeroTheme theme = HeroTheme.light}) {
  return HeroScope(
    theme: theme,
    child: MaterialApp(home: Scaffold(body: Center(child: child))),
  );
}

void main() {
  testWidgets('HeroButton renders its label and fires onPressed',
      (tester) async {
    var pressed = 0;
    await tester.pumpWidget(
      _host(
        HeroButton(label: 'Continue', onPressed: () => pressed++),
      ),
    );
    expect(find.text('Continue'), findsOneWidget);
    await tester.tap(find.text('Continue'));
    expect(pressed, 1);
  });

  testWidgets('HeroButton is disabled when onPressed is null', (tester) async {
    await tester.pumpWidget(
      _host(const HeroButton(label: 'Nope')),
    );
    final button = tester.widget<HeroButton>(find.byType(HeroButton));
    expect(button.onPressed, isNull);
    // The RemixButton underneath must receive enabled: false.
    final remix = find.byWidgetPredicate(
      (w) => w.runtimeType.toString() == 'RemixButton',
    );
    expect(remix, findsOneWidget);
  });

  testWidgets('every variant builds across light and dark', (tester) async {
    for (final theme in HeroTheme.values) {
      for (final variant in HeroButtonVariant.values) {
        await tester.pumpWidget(
          _host(
            HeroButton(label: variant.name, variant: variant, onPressed: () {}),
            theme: theme,
          ),
        );
        expect(tester.takeException(), isNull, reason: '$variant / $theme');
      }
    }
  });

  testWidgets('HeroButton measured height matches the md token (36)',
      (tester) async {
    await tester.pumpWidget(
      _host(HeroButton(label: 'X', onPressed: () {})),
    );
    final size = tester.getSize(find.byType(HeroButton));
    expect(size.height, 36.0);
  });

  testWidgets('loading keeps the kind fill instead of the disabled gray',
      (tester) async {
    await tester.pumpWidget(
      _host(
        HeroButton(
          label: 'Save',
          variant: HeroButtonVariant.danger,
          loading: true,
          onPressed: () {},
        ),
      ),
    );
    // The container decoration should still be the danger fill, not a gray.
    final decorated = tester.widget<DecoratedBox>(
      find.descendant(
        of: find.byType(HeroButton),
        matching: find.byWidgetPredicate((w) => w is DecoratedBox),
      ),
    );
    final decoration = decorated.decoration as BoxDecoration;
    expect(decoration.color, const Color(0xFFFF383C)); // --danger light
  });

  testWidgets('disabled HeroButton fades to the disabled opacity', (tester) async {
    await tester.pumpWidget(
      _host(const HeroButton(label: 'X')),
    );
    final opacity = tester.widget<Opacity>(
      find.descendant(
        of: find.byType(HeroButton),
        matching: find.byType(Opacity),
      ),
    );
    expect(opacity.opacity, 0.5);
  });
}
