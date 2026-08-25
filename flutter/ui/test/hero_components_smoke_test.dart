import 'package:congelado_hero_ui/congelado_hero_ui.dart';
import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

/// Pumps a screen with every Hero* component inside HeroScope + MaterialApp
/// (the host the README documents). Any token-handle typo or recipe error
/// surfaces here as a build exception.
Widget _host(Widget child, {HeroTheme theme = HeroTheme.light}) {
  return HeroScope(
    theme: theme,
    child: MaterialApp(
      home: Scaffold(
        body: SingleChildScrollView(child: child),
      ),
    ),
  );
}

void main() {
  testWidgets('every Hero component builds in light and dark', (tester) async {
    for (final theme in HeroTheme.values) {
      await tester.pumpWidget(
        _host(
          Column(
            children: [
              HeroButton(label: 'B', onPressed: () {}),
              HeroButton(
                label: 'B loading',
                loading: true,
                onPressed: () {},
              ),
              const HeroCard(child: Text('card')),
              const HeroCardTitle('title'),
              const HeroCardDescription('description'),
              HeroInput(hintText: 'input'),
              const HeroInput(error: true),
              const HeroChip(label: 'chip'),
              const HeroChip(label: 'chip solid', variant: HeroChipVariant.solid),
              const HeroBadge(label: 'badge'),
              HeroTabs(
                selectedTabId: 'a',
                tabs: const [
                  HeroTab(id: 'a', label: 'A', child: Text('A')),
                  HeroTab(id: 'b', label: 'B', child: Text('B')),
                ],
              ),
              HeroProgress(value: 0.5),
              const HeroSpinner(),
              HeroSwitch(selected: true, onChanged: (_) {}),
              HeroCheckbox(selected: true, onChanged: (_) {}),
              HeroRadioGroup<String>(
                groupValue: 'a',
                onChanged: (_) {},
                child: const Row(
                  children: [HeroRadio(value: 'a'), HeroRadio(value: 'b')],
                ),
              ),
              const HeroDivider(),
              const HeroAvatar(label: 'JD'),
              const HeroTooltip(message: 'tip', child: Text('hover me')),
              const SizedBox(width: 100, child: HeroSkeleton(height: 12)),
            ],
          ),
          theme: theme,
        ),
      );
      expect(tester.takeException(), isNull, reason: 'theme=$theme');
      await tester.pump();
      expect(tester.takeException(), isNull, reason: 'theme=$theme (after pump)');
    }
  });

  testWidgets('showHeroModal opens a dialog and dismisses it', (tester) async {
    await tester.pumpWidget(
      _host(
        Builder(
          builder: (context) => HeroButton(
            label: 'open',
            onPressed: () => showHeroModal<void>(
              context,
              title: 'Title',
              description: 'Description',
              builder: (context) => const Text('modal body'),
            ),
          ),
        ),
      ),
    );
    await tester.tap(find.text('open'));
    await tester.pumpAndSettle();
    expect(find.text('Title'), findsOneWidget);
    // Tap the barrier off the dialog (the dialog covers the screen centre).
    await tester.tapAt(const Offset(5, 5));
    await tester.pumpAndSettle();
    expect(find.text('Title'), findsNothing);
  });
}
